/*
 * This file is part of Embedded-IDE
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
#include "appconfig.h"
#include "findinfilesdialog.h"
#include "ui_findinfilesdialog.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QListView>
#include <QMenu>
#include <QStandardItemModel>
#include <QWidgetAction>

#include <Qsci/qsciscintilla.h>

struct FilePos {
    int line{ 0 };
    int column{ 0 };
    QString path;

    FilePos() = default;
    FilePos(int l, int c, const QString& p): line(l), column(c), path(p) {}
};

Q_DECLARE_METATYPE(FilePos)

const QStringList STANDARD_FILTERS =
        { "*.*", "*.c", "*.cpp", "*.h", "*.hpp", "*.txt" };

FindInFilesDialog::FindInFilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindInFilesDialog)
{
    ui->setupUi(this);
    AppConfig::fixIconTheme(this);

    auto model = new QStandardItemModel(this);
    ui->treeView->setModel(model);
    auto protoItem = new QStandardItem();
    protoItem->setFont(AppConfig::instance().loggerFont());
    model->setItemPrototype(protoItem);
    ui->textToFind->addMenuActions(QHash<QString, QString>{
                                      { tr("Regular Expression"), "regex" },
                                      { tr("Case Sensitive"), "case" },
                                      { tr("Wole Words"), "wword" },
                                  });
    ui->labelStatus->setText(tr("Ready"));

    auto filterMenu = new QMenu(this);
    auto listViewAction = new QWidgetAction(filterMenu);
    auto filterListView = new QListView(this);
    filterListView->setEditTriggers(QListView::NoEditTriggers);
    auto filterModel = new QStandardItemModel(filterListView);
    for(const auto& e: STANDARD_FILTERS) {
        auto i = new QStandardItem(e);
        i->setCheckable(true);
        filterModel->appendRow(i);
    }
    connect(filterModel, &QStandardItemModel::itemChanged, [this](QStandardItem *item) {
        QString filter = ui->textFilePattern->text();
        if (item->checkState() == Qt::Checked) {
            if (!filter.isEmpty())
                filter.append(", ");
            filter.append(item->text());
        } else {
            filter.remove(QRegularExpression(QString(R"(\,?\s?%1,?)")
                                             .arg(QRegularExpression::escape(item->text()))));
        }
        ui->textFilePattern->setText(filter.trimmed());
    });
    filterModel->item(0, 0)->setCheckState(Qt::Checked);
    filterListView->setModel(filterModel);
    listViewAction->setDefaultWidget(filterListView);
    filterMenu->addAction(listViewAction);
    ui->buttonSelectfilePattern->setMenu(filterMenu);
    ui->buttonSelectfilePattern->setPopupMode(QToolButton::InstantPopup);

    connect(ui->treeView, &QTreeView::activated,
            [this, model](const QModelIndex& index) {
        auto item = model->itemFromIndex(index);
        if (item) {
            auto pos = item->data().value<FilePos>();
            emit queryToOpen(pos.path, pos.line, pos.column);
            accept();
        }
    });

    auto doc = new QsciScintilla(this);
    doc->hide();
    connect(ui->buttonFind, &QToolButton::clicked, [this, model, doc]() {
        setProperty("onProcessLoop", true);
        model->clear();
        ui->buttonStop->setEnabled(true);
        ui->buttonFind->setDisabled(true);
        QStringList filters = ui->textFilePattern->text().split(",")
                .replaceInStrings(QRegularExpression(R"(^\s+)"), QString())
                .replaceInStrings(QRegularExpression(R"(\s+$)"), QString());
        QString textToFind = ui->textToFind->text();
        bool isRegex = ui->textToFind->isPropertyChecked("regex");
        bool isCS = ui->textToFind->isPropertyChecked("case");
        bool isWWord = ui->textToFind->isPropertyChecked("wword");
        QDirIterator it(ui->textDirectory->text(), filters,
                        QDir::NoFilter, QDirIterator::Subdirectories);
        while (it.hasNext() && property("onProcessLoop").toBool()) {
            QCoreApplication::processEvents();
            if (!property("onProcessLoop").toBool())
                break;
            ui->labelStatus->setText(tr("Scanning file:"));
            ui->labelFilename->setText(QFontMetrics(ui->labelStatus->font())
                                       .elidedText(it.next(), Qt::ElideLeft, ui->labelStatus->width()));
            auto info = it.fileInfo();
            if (!info.isFile())
                continue;
            QFile file(info.absoluteFilePath());
            if (!file.open(QFile::ReadOnly))
                continue;
            if (!doc->read(&file))
                continue;
            if (!doc->findFirst(textToFind, isRegex, isCS, isWWord, false, true, -1, -1, false, false))
                continue;
            auto fileItem = model->itemPrototype()->clone();
            fileItem->setText(info.absoluteFilePath().remove(ui->textDirectory->text() + QDir::separator()));
            model->appendRow(fileItem);
            do {
                int line, column;
                doc->getCursorPosition(&line, &column);
                QString textInFile = doc->text(line);
                auto posItem = model->itemPrototype()->clone();
                posItem->setText(tr("Line %1 Char %2: %3")
                                 .arg(QString("%1").arg(line).leftJustified(8, ' '))
                                 .arg(QString("%1").arg(column).leftJustified(8, ' '))
                                 .arg(textInFile));
                posItem->setData(QVariant::fromValue(FilePos{ line, column, info.absoluteFilePath() }));
                posItem->setData(Qt::AlignBaseline, Qt::TextAlignmentRole);
                fileItem->appendRow(posItem);
                QCoreApplication::processEvents();
            } while (doc->findNext() && property("onProcessLoop").toBool());
        }
        ui->buttonStop->setDisabled(true);
        ui->buttonFind->setEnabled(true);
        ui->treeView->expandAll();
        ui->labelStatus->setText(tr("Done"));
        ui->labelFilename->clear();
        setProperty("onProcessLoop", false);
    });
    connect(this, &QDialog::finished, [this]() {
        setProperty("onProcessLoop", false);
    });
    connect(ui->buttonStop, &QToolButton::clicked, [this]() {
        setProperty("onProcessLoop", false);
    });

    connect(ui->buttonChoseDirectory, &QToolButton::clicked, [this]() {
        QString path = QFileDialog::getExistingDirectory(this,
                                                         tr("Select directory"),
                                                         ui->textDirectory->text());
        if (!path.isEmpty())
            ui->textDirectory->setText(path);
    });
}

FindInFilesDialog::~FindInFilesDialog()
{
    delete ui;
}

QString FindInFilesDialog::findPath() const
{
    return ui->textDirectory->text();
}

void FindInFilesDialog::setFindPath(const QString &path)
{
    ui->textDirectory->setText(path);
}

void FindInFilesDialog::closeEvent(QCloseEvent *)
{
    setProperty("onProcessLoop", false);
}
