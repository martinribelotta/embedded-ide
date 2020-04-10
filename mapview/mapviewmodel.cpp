/*
 * This file is part of mapview, a component of Embedded-IDE
 *
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "mapviewmodel.h"

#include <QApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <cstddef>
#include <cstdint>

#include <QtDebug>
#include <utility>

template<typename T>
static QString toHex(T v) {
    return QString("0x%1").arg(v, sizeof(v) * 2, 16, QChar('0'));
}

BarItemDelegate::BarItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

BarItemDelegate::~BarItemDelegate() = default;

QStandardItem *setMyData(QStandardItem *i, const QVariant& v) {
    i->setData(v, Qt::UserRole);
    return i;
}

QStyleOptionProgressBar BarItemDelegate::options(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    double percent = index.data(Qt::UserRole).toDouble();
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.progress = int(percent);
    progressBarOption.text = QString("%1%").arg(percent, 3, 'f', 2, QChar('0'));
    progressBarOption.textVisible = true;
    return progressBarOption;
}

void BarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto progressBarOption = options(option, index);
    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}

QSize BarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto progressBarOption = options(option, index);
    auto textSize = option.fontMetrics.boundingRect(progressBarOption.text).size();
    auto widgetSize = QApplication::style()->sizeFromContents(QStyle::CT_ProgressBar, &progressBarOption, textSize) * 1;
    return widgetSize;
}

const int MapViewModel::PERCENT_COLUMN = 4;

MapViewModel::MapViewModel(QObject *parent): QStandardItemModel(parent)
{
    setHorizontalHeaderLabels({
                                  tr("Name"),
                                  tr("Address"),
                                  tr("Size"),
                                  tr("Used"),
                                  tr("Percent used"),
                                  tr("Attributes")
                              });
}

MapViewModel::~MapViewModel()
= default;

struct Symbol_t {
    QString name;
    size_t base;
};

struct MemSection_t {
    QString name;
    size_t base{ 0 };
    size_t size{ 0 };
    size_t lma{ 0 };

    QList<Symbol_t> symbolList;

    MemSection_t() = default;
    MemSection_t(QString  n, size_t b, size_t s, size_t l):
        name(std::move(n)), base(b), size(s), lma(l) {}

    inline bool into(size_t addr) const { return (addr >= base) && (addr <= (base + size)); }
};

struct MemRegion_t {
    QString name;
    size_t base{};
    size_t size{};
    QString attr;
    size_t used{};

    qreal usedPercent() const {
        return qreal(used) * 100.0 / qreal(size);
    }

    QList<MemSection_t> sections;

    MemRegion_t() = default;
    MemRegion_t(QString  n, size_t b, size_t s, QString  a):
        name(std::move(n)), base(b), size(s), attr(std::move(a)), used(0) {}

    inline bool into(size_t addr) const { return (addr >= base) && (addr <= (base + size)); }
    inline bool into(const MemSection_t& s) const { return into(s.base) && into(s.base + s.size); }
};

bool MapViewModel::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << file.errorString();
        return false;
    }
    auto text = QTextStream(&file).readAll();

    QList<MemRegion_t> memoryRegions;
    QList<MemSection_t*> sectionList;

    int memSectionIdx = text.indexOf("Memory Configuration");
    int linkSectionIdx = text.indexOf("Linker script and memory map");
    if (memSectionIdx) {
        QRegularExpression MEM_RE(R"(^([a-zA-Z_][a-zA-Z0-9_]*)\s+(0x[\da-fA-F]+)\s+(0x[\da-fA-F]+)\s*([rwx]*)$)",
                                  QRegularExpression::MultilineOption);
        auto it = MEM_RE.globalMatch(text, memSectionIdx);
        while (it.hasNext()) {
            auto m = it.next();
            memoryRegions.append({
                m.captured(1),
                size_t(m.captured(2).remove(0, 2).toLongLong(nullptr, 16)),
                size_t(m.captured(3).remove(0, 2).toLongLong(nullptr, 16)),
                m.captured(4)
            });
        }
    }
    if (linkSectionIdx) {
        QRegularExpression ignoredRe[] = {
            QRegularExpression{ R"(^LOAD\s)", QRegularExpression::MultilineOption },
            QRegularExpression{ R"(^(?:START|END) GROUP\s)", QRegularExpression::MultilineOption },
            QRegularExpression{ R"(^LOAD\s)", QRegularExpression::MultilineOption },
            QRegularExpression{ R"(^\s+0x[\da-f]+\s+\.)", QRegularExpression::MultilineOption },
            QRegularExpression{ R"(^\s+\*\()", QRegularExpression::MultilineOption },
            QRegularExpression{ R"(^\sFILL\s)", QRegularExpression::MultilineOption },
        };
        QRegularExpression SYM_DECL(R"(^\s+(0x[\da-f]+)\s+([a-z_\$\:][a-z_\$\:\d]*.*$))",
                                    QRegularExpression::MultilineOption);
        QRegularExpression SEC_DECL(R"(^(\.[a-zA-Z_][_a-zA-Z\d\.]+)\s+(0x[\da-fA-F]+)\s+(0x[\da-fA-F]+)(?:\s+load address\s+(0x[\da-fA-F]+))?)",
                                    QRegularExpression::MultilineOption);
        QRegularExpression LINE_RE(R"(^.*$)", QRegularExpression::MultilineOption);
        auto it = LINE_RE.globalMatch(text, linkSectionIdx);
        while (it.hasNext()) {
            auto line = it.next().captured();
            bool ignore = false;
            for(const QRegularExpression& re: ignoredRe)
                if (re.match(line).hasMatch())
                    { ignore = true; break; }
            if (ignore)
                continue;
            auto sectionDeclMatch = SEC_DECL.match(line);
            if (sectionDeclMatch.hasMatch()) {
                MemSection_t s = {
                    sectionDeclMatch.captured(1),
                    size_t(sectionDeclMatch.captured(2).toULongLong(nullptr, 16)),
                    size_t(sectionDeclMatch.captured(3).toULongLong(nullptr, 16)),
                    size_t(sectionDeclMatch.captured(4).toULongLong(nullptr, 16)),
                };
                for(MemRegion_t& r: memoryRegions)
                    if (r.into(s)) {
                        r.used += s.size;
                        r.sections.append(s);
                        sectionList.append(&r.sections.last());
                    }
                continue;
            }
            auto symbolDeclMatch = SYM_DECL.match(line);
            if (symbolDeclMatch.hasMatch()) {
                for(MemSection_t *s: sectionList) {
                    Symbol_t sym = {
                        symbolDeclMatch.captured(2),
                        size_t(symbolDeclMatch.captured(1).toULongLong(nullptr, 16)),
                    };
                    if (s->into(sym.base))
                        s->symbolList.append(sym);
                }
                continue;
            }
        }
    }

    for(const MemRegion_t& r: memoryRegions) {
        QList<QStandardItem*> regionItems = {
            new QStandardItem(r.name),
            new QStandardItem(toHex(r.base)),
            new QStandardItem(toHex(r.size)),
            new QStandardItem(QString("%1").arg(r.used)),
            setMyData(new QStandardItem(QString("%1").arg(r.usedPercent())), r.usedPercent()),
            new QStandardItem(r.attr),
        };
        for(const auto& sec: r.sections) {
            QList<QStandardItem*> sectionItems = {
                new QStandardItem(sec.name),
                new QStandardItem(toHex(sec.base)),
                new QStandardItem(tr("%1 (Load %2)").arg(toHex(sec.size)).arg(toHex(sec.lma))),
            };
            for(const auto& sym: sec.symbolList) {
                QList<QStandardItem*> symbolItems = {
                    new QStandardItem(sym.name),
                    new QStandardItem(toHex(sym.base)),
                };
                sectionItems[0]->appendRow(symbolItems);
            }
            regionItems[0]->appendRow(sectionItems);
        }
        appendRow(regionItems);
    }
    return true;
}
