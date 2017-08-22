#include "gdbstartdialog.h"
#include "ui_gdbstartdialog.h"

#include <QFileDialog>
#include <QListView>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardItemModel>

#include "appconfig.h"

GDBStartDialog::GDBStartDialog(const QString& currentProjectPath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GDBStartDialog),
    m_currentProjectPath(currentProjectPath)
{
    ui->setupUi(this);
    ui->textEditCommands->setFont(AppConfig::systemMonoFont());
}

GDBStartDialog::~GDBStartDialog()
{
    delete ui;
}

GDBStartDialog::GdbConfig GDBStartDialog::config() const
{
    return GdbConfig{
        ui->editGdbExecutable->text(),
        ui->editProgramExecutable->text(),
        ui->textEditCommands->toPlainText().split(QRegularExpression(R"(\r?\n)"))
    };
}

void GDBStartDialog::on_buttonLoadPredefinedConfig_clicked()
{
    QString name = QFileDialog::getOpenFileName(window(), tr("Open GDB Configuration file"),
                                                QDir::homePath(),
                                                tr("GDB Configuration files (*.gdbconf);;"
                                                   "All files (*)"));
    if (!name.isEmpty())
        ui->comboPredefinedConfig->setCurrentText(name);
}

void GDBStartDialog::on_buttonLoadGDBExecutable_clicked()
{
    QString name = QFileDialog::getOpenFileName(window(), tr("Open GDB executable"),
                                                QDir::homePath(),
                                                tr("GDB Configuration files (*gdb*);;"
                                                   "All files (*)"));
    if (!name.isEmpty())
        ui->editGdbExecutable->setText(name);
}

void GDBStartDialog::on_buttonLoadProgramExecutable_clicked()
{
    QString name = QFileDialog::getOpenFileName(window(), tr("Open executable to debug"),
                                                QDir::homePath(), tr(
#ifdef Q_OS_WIN
                                                    "Executables (*.exe);;"
#endif
                                                    "All files (*)"));
    if (!name.isEmpty())
        ui->editGdbExecutable->setText(name);
}

#ifdef Q_OS_UNIX
#define EXEC_MIMETYPE "application/x-executable"
#else
#define EXEC_MIMETYPE "application/x-msdownload"
#endif

static QMimeDatabase mimeDb;

static void findExecutables(QStandardItemModel *m, const QDir& path) {
    auto entries = path.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    foreach(QFileInfo info, entries) {
        if (info.isDir()) {
            findExecutables(m, QDir(info.absoluteFilePath()));
        } else {
            QFile f(info.absoluteFilePath());
            QMimeType type = mimeDb.mimeTypeForFileNameAndData(info.absoluteFilePath(), &f);
            if (type.inherits(EXEC_MIMETYPE)) {
                QStandardItem *item = new QStandardItem();
                item->setText(info.fileName());
                item->setData(QVariant(info.absoluteFilePath()));
                m->appendRow(QList<QStandardItem*>() << item);
            }
        }
    }
}

void GDBStartDialog::on_buttonFindProgramExecutable_clicked()
{
    QDialog dialog;
    QListView *view = new QListView(&dialog);
    QHBoxLayout *layout = new QHBoxLayout(&dialog);
    layout->addWidget(view);
    QStandardItemModel *model = new QStandardItemModel(view);
    connect(view, &QListView::clicked, [view, model, &dialog](const QModelIndex& idx) {
        Q_UNUSED(idx);
        dialog.accept();
    });
    findExecutables(model, QDir(m_currentProjectPath));
    view->setModel(model);
    dialog.exec();
    auto itemIdx = view->currentIndex();
    if (itemIdx.isValid()) {
        QStandardItem *sel = model->itemFromIndex(itemIdx);
        if (sel) {
            QString path = sel->data().toString();
            ui->editProgramExecutable->setText(path);
        }
    }
}
