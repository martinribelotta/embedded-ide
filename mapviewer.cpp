#include "codeeditor.h"
#include "mapviewer.h"
#include "ui_mapviewer.h"

#include <limits>

#include <QApplication>
#include <QFile>
#include <QHeaderView>
#include <QProgressBar>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <QtDebug>

static const QLatin1String MEMORY_MAP_HEADER("Memory Configuration");
static const QLatin1String REGEX_SYM(R"(^\s+0x([\da-f]+)\s+(.+?)$)");
static const QLatin1String REGEX_MEM(R"(^(?P<name>\S+)\s+)"
                                     R"((?P<origin>0x[0-9a-fA-F]+)\s+)"
                                     R"((?P<length>0x[0-9a-fA-F]+)\s+)"
                                     R"((?P<attr>[xrw]+)$)");
static const QLatin1String REGEX_SEC(R"(^(?!\.debug|\.comment|.*?attributes)(?P<name>\.\S+)\s+)"
                                     R"((?P<vma>0x[a-f0-9]+)\s+)"
                                     R"((?!0x0)(?P<size>0x[a-f0-9]+))"
                                     R"((?:\s+load address (?P<lma>0x[a-z0-9]+))?)");
static const QLatin1String REGEX_TRU(R"(^ (\.[a-z0-9_]+)[ \t])"
                                     R"(+0x([0-9a-f]+)[ \t])"
                                     R"(+0x([0-9a-f]+)[ \t])"
                                     R"(+(\S+)$)");

#if 1
struct MemoryChunk {
    QString name;
    uint32_t vma;
    uint32_t lma;
    uint32_t size;
    uint32_t pad;

    bool isRelocatable() const {
        return vma == lma;
    }

    uint32_t hi_vma() const {
        return vma + size;
    }
    uint32_t hi_lma() const {
        return lma + size;
    }
};

inline QDebug operator<<(QDebug dbg, const MemoryChunk& chunk) {
    dbg << chunk.name
        << ":{ vma:" << chunk.vma
        << "size:" << chunk.size
        << "size:" << chunk.lma
        << " }";
    return dbg;
}

struct MemoryRegion {
    QString name;
    uint32_t base;
    uint32_t size;
    QString attr;
    uint32_t used;
    uint32_t pad;

    uint32_t upper() const {
        return base + size;
    }

    bool inRegion(const MemoryChunk& chunk) const {
        return ((chunk.vma >= base) && (chunk.hi_vma() <= upper())) ||
               ((chunk.lma >= base) && (chunk.hi_lma() <= upper()));
    }
};

inline QDebug operator<<(QDebug dbg, const MemoryRegion& m) {
    dbg << m.name
        << ":{ base:" << m.base
        << "size:" << m.size
        << "attr:" << m.attr
        << " }";
    return dbg;
}
#endif

struct LinkerSymbol {
    uint32_t value;
    uint8_t pad[4];
    QString expr;
    LinkerSymbol(uint32_t v, const QString& e) : value(v), expr(e) { }
};

struct TranslationUnit {
    QString section;
    uint32_t addr;
    uint32_t size;
    QString path;
    QList<LinkerSymbol> symbols;

    TranslationUnit() : addr(UINT32_MAX), size(0) { }

    TranslationUnit(const QString& s, uint32_t a, uint32_t z, const QString& p):
        section(s), addr(a), size(z), path(p) {}

    bool isEmpty() const {
        return size == 0 && symbols.isEmpty();
    }

    bool isNull() const {
        return section.isNull() && addr == UINT32_MAX && size == 0 && path.isNull();
    }
};

struct Section {
    uint32_t base;
    uint32_t load;
    uint32_t size;
    uint8_t pad[4];
    QList<TranslationUnit> tru;

    Section() : base(0), load(0), size(0) { tru.append(TranslationUnit()); }

    Section(u_int32_t vma, uint32_t lma, uint32_t z) : base(vma), load(lma), size(z) {
        tru.append(TranslationUnit());
    }

    bool isRelocatable() const {
        return base == load;
    }

    uint32_t hi_addr() const {
        return base + size;
    }

    uint32_t hi_load() const {
        return load + size;
    }

    bool inRegion(const MemoryRegion& r) const {
        return (base >= r.base && base <= r.upper()) ||
               (load >= r.base && load <= r.upper());
    }
};

struct MapData {
    QList<MemoryRegion> memoryRegions;
    QHash<QString, Section> memoryMap;
};

static QHash<QString, Section> parseMemoryMap(QTextStream& stream, QList<MemoryRegion> &mr) {
    QHash<QString, Section> sectionList;
    QRegularExpression symbolRe(REGEX_SYM, QRegularExpression::CaseInsensitiveOption);
    QRegularExpression sectionRe(REGEX_SEC, QRegularExpression::CaseInsensitiveOption);
    QRegularExpression truRe(REGEX_TRU, QRegularExpression::CaseInsensitiveOption);
    auto currentSection = sectionList.insert("", Section());
    while(!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.startsWith("LOAD ")) {
            continue;
        }
        QRegularExpressionMatch m;
        m = sectionRe.match(line);
        if (m.hasMatch()) {
            auto secName = m.captured("name");
            auto secVma = m.captured("vma").remove(0, 2).toUInt(nullptr, 16);
            auto secSize = m.captured("size").remove(0, 2).toUInt(nullptr, 16);
            auto secLma = m.lastCapturedIndex() == 4? m.captured("lma").remove(0, 2).toUInt(nullptr, 16) : secVma;
            currentSection = sectionList.insert(secName, Section{ secVma, secLma, secSize });
            for (int i=0; i<mr.size(); i++) {
                if (currentSection.value().inRegion(mr.at(i)))
                    mr[i].used += currentSection.value().size;
            }
            continue;
        }
        m = truRe.match(line);
        if (m.hasMatch()) {
            auto section = m.captured(1);
            auto addr = m.captured(2).toUInt(nullptr, 16);
            auto size = m.captured(3).toUInt(nullptr, 16);
            auto path = m.captured(4);
            currentSection.value().tru.append(TranslationUnit(section, addr, size, path));
            continue;
        }
        m = symbolRe.match(line);
        if (m.hasMatch()) {
            uint32_t val = m.captured(1).toUInt(nullptr, 16);
            QString expr = m.captured(2);
            currentSection.value().tru.back().symbols.append(LinkerSymbol(val, expr));
            continue;
        }
    }
    return sectionList;
}

static MapData parseMemoryConfiguration(QTextStream& stream) {
    MapData data;
    QRegularExpression re(R"(^([a-z0-9_]+)\s+0x([\da-f]+)\s+0x([\d+a-f]+)\s+([rwx]+)$)",
                          QRegularExpression::CaseInsensitiveOption);
    while(!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.startsWith("Linker script and memory map")) {
            data.memoryMap = parseMemoryMap(stream, data.memoryRegions);
        } else {
            auto m = re.match(line);
            if (m.hasMatch()) {
                QString name = m.captured(1);
                uint32_t addr = m.captured(2).toUInt(nullptr, 16);
                uint32_t size = m.captured(3).toUInt(nullptr, 16);
                QString attr = m.captured(4);
                data.memoryRegions.append(MemoryRegion{ name, addr, size, attr, 0, 0 });
            }
        }
    }
    return data;
}

static void parseMap(QTextStream &stream) {
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.startsWith("Memory Configuration"))
            parseMemoryConfiguration(stream);
    }
}

static bool readMemoryMap(const QString &text,
                          QList<MemoryRegion> &memoryRegions,
                          QList<MemoryChunk> &memoryChuncks) {
    int memHeaderIdx = text.indexOf(MEMORY_MAP_HEADER);
    if (memHeaderIdx == -1)
        return false;
    auto re_mem = QRegularExpression(REGEX_MEM, QRegularExpression::CaseInsensitiveOption |
                                                QRegularExpression::MultilineOption);
    auto mem_matchs = re_mem.globalMatch(text, memHeaderIdx);
    while (mem_matchs.hasNext()){
        auto m = mem_matchs.next();
        auto memName = m.captured("name");
        auto memOrg = m.captured("origin").remove(0, 2).toUInt(nullptr, 16);
        auto memLen = m.captured("length").remove(0, 2).toUInt(nullptr, 16);
        auto memAttr = m.captured("attr");
        memoryRegions.append(MemoryRegion({ memName, memOrg, memLen, memAttr, 0, 0 }));
    }

    auto re_sec = QRegularExpression(REGEX_SEC, QRegularExpression::CaseInsensitiveOption |
                                                QRegularExpression::MultilineOption);
    auto sec_matchs = re_sec.globalMatch(text, memHeaderIdx);
    while (sec_matchs.hasNext()) {
        auto m = sec_matchs.next();
        auto secName = m.captured("name");
        auto secVma = m.captured("vma").remove(0, 2).toUInt(nullptr, 16);
        auto secSize = m.captured("size").remove(0, 2).toUInt(nullptr, 16);
        auto secLma = m.lastCapturedIndex() == 4? m.captured("lma").remove(0, 2).toUInt(nullptr, 16) : secVma;
        auto chunk = MemoryChunk({ secName, secVma, secLma, secSize, 0 });
        memoryChuncks.append(chunk);
        qDebug() << chunk;
        for(int i=0; i<memoryRegions.count(); i++) {
            MemoryRegion &r = memoryRegions[i];
            if (r.inRegion(chunk))
                r.used += chunk.size;
        }
    }
    qSort(memoryRegions.begin(), memoryRegions.end(),
          [](const MemoryRegion& a, const MemoryRegion& b) -> bool { return a.used > b.used; });
    return true;
}

static const uint HUMAN_Kb = 1024;
static const uint HUMAN_Mb = 1024 * HUMAN_Kb;
static const uint HUMAN_Gb = 1024 * HUMAN_Mb;

static QString unscaledPrefix(uint32_t count, const QString& pluralPrefix, const QString& singularPrefix)
{
    return QString("%1 %2").arg(count).arg(count != 1? pluralPrefix : singularPrefix);
}

static QString _toHumanReadable(uint32_t count, uint32_t *used, const QString& pluralPrefix, const QString& singularPrefix)
{
    Q_UNUSED(used);
#define IF_HUMAN(c, f) \
    if (c > HUMAN_##f##b) \
        return QString("%1 " #f "%2").arg(*used = c / HUMAN_##f##b).arg(pluralPrefix);
    IF_HUMAN(count, G);
    IF_HUMAN(count, M);
    IF_HUMAN(count, K);
    return unscaledPrefix(count, pluralPrefix, singularPrefix);
}

static QString toHumanReadable(uint32_t count, const QString& pluralPrefix, const QString& singularPrefix)
{
    uint32_t scaledCount = count;
    QString human = _toHumanReadable(count, &scaledCount, pluralPrefix, singularPrefix);
    if (scaledCount == count)
        return human;
    else
        return QString("%1 (%2)").arg(human, unscaledPrefix(count, pluralPrefix, singularPrefix));
}

class BarItemDelegate: public QStyledItemDelegate {
public:
    BarItemDelegate(QObject *parent = Q_NULLPTR) : QStyledItemDelegate(parent) {}
    virtual ~BarItemDelegate();

    inline QStyleOptionProgressBar options(const QStyleOptionViewItem& option,
                                           const QModelIndex& index) const {
        double percent = index.data(Qt::UserRole).toDouble();
        // qDebug() << "" << percent;
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = int(percent);
        progressBarOption.text = QString("%1%").arg(percent, 3, 'f', 2, QChar('0'));
        progressBarOption.textVisible = true;
        return progressBarOption;
    }

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        auto progressBarOption = options(option, index);
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const
    {
        auto progressBarOption = options(option, index);
        auto textSize = option.fontMetrics.boundingRect(progressBarOption.text).size();
        auto widgetSize = QApplication::style()->sizeFromContents(QStyle::CT_ProgressBar, &progressBarOption, textSize) * 2;
        return widgetSize;
    }
};

BarItemDelegate::~BarItemDelegate()
{
}

MapViewer::MapViewer(QWidget *parent) :
    QWidget (parent),
    ui(new Ui::MapViewer)
{
    ui->setupUi(this);
    ui->memoryTable->setModel(new QStandardItemModel(this));
    ui->memoryTable->setEditTriggers(QTableView::NoEditTriggers);
    ui->memoryTable->setAlternatingRowColors(true);
    ui->memoryTable->verticalHeader()->hide();
    ui->memoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->memoryTable->horizontalHeader()->setStretchLastSection(true);
    ui->memoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->memoryTable->setItemDelegateForColumn(4, new BarItemDelegate(this));
}

MapViewer::~MapViewer()
{
    delete ui;
}

bool MapViewer::load(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
#if 1
        QTextStream stream(&f);
        MapData data;
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("Memory Configuration")) {
                data = parseMemoryConfiguration(stream);
            }
        }
        auto m = qobject_cast<QStandardItemModel*>(ui->memoryTable->model());
        m->clear();
        m->setHorizontalHeaderLabels(QStringList({tr("Memory"),
                                                  tr("Base address"),
                                                  tr("Size"),
                                                  tr("Used"),
                                                  tr("Percent"),
                                                  tr("Attributes"),
                                                 }));
        qSort(data.memoryRegions.begin(), data.memoryRegions.end(),
              [](const MemoryRegion& a, const MemoryRegion& b) -> bool { return a.used > b.used; });
        foreach(auto r, data.memoryRegions) {
            qDebug() << r;
            QList<QStandardItem*> items;
            items += new QStandardItem(r.name);
            items += new QStandardItem(QString("0x%1").arg(r.base, 8, 16, QChar('0')));
            items += new QStandardItem(toHumanReadable(r.size, "bytes", "byte"));
            items += new QStandardItem(toHumanReadable(r.used, "bytes", "byte"));
            items += new QStandardItem();
            items += new QStandardItem(r.attr);
            items[4]->setData((r.used * 100.0) / r.size, Qt::UserRole);
            m->appendRow(items);
        }
        ui->memoryTable->resizeColumnsToContents();
        ui->memoryTable->resizeRowsToContents();
        return true;
#else
        QList<MemoryRegion> memoryRegions;
        QList<MemoryChunk> memoryChuncks;
        if (readMemoryMap(f.readAll(), memoryRegions, memoryChuncks)) {
            auto m = qobject_cast<QStandardItemModel*>(ui->memoryTable->model());
            m->clear();
            m->setHorizontalHeaderLabels(QStringList({tr("Memory"),
                                                      tr("Base address"),
                                                      tr("Size"),
                                                      tr("Used"),
                                                      tr("Percent"),
                                                      tr("Attributes"),
                                                     }));
            foreach(auto r, memoryRegions) {
                qDebug() << r;
                QList<QStandardItem*> items;
                items += new QStandardItem(r.name);
                items += new QStandardItem(QString("0x%1").arg(r.base, 8, 16, QChar('0')));
                items += new QStandardItem(toHumanReadable(r.size, "bytes", "byte"));
                items += new QStandardItem(toHumanReadable(r.used, "bytes", "byte"));
                items += new QStandardItem();
                items += new QStandardItem(r.attr);
                items[4]->setData((r.used * 100.0) / r.size, Qt::UserRole);
                m->appendRow(items);
            }
            ui->memoryTable->resizeColumnsToContents();
            ui->memoryTable->resizeRowsToContents();
            return true;
        }
#endif
    }
    return false;
}
