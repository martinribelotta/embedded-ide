#include "gdbstartdialog.h"
#include "ui_gdbstartdialog.h"

#include <QFileDialog>
#include <QListView>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardItemModel>

#include "appconfig.h"

GDBStartDialog::GDBStartDialog(const MakefileInfo &info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GDBStartDialog),
    m_info(info)
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
                                                m_info.workingDir, tr(
#ifdef Q_OS_WIN
                                                    "Executables (*.exe);;"
#endif
                                                    "All files (*)"));
    if (!name.isEmpty())
        ui->editProgramExecutable->setText(name);
}

static const QStringList EXEC_MIMETYPE_LIST = {
    "application/x-executable",
#ifdef Q_OS_WIN
    "application/x-msdownload",
#endif
};

static QMimeDatabase mimeDb;

static bool isExecMimetype(const QMimeType& type) {
    for(auto a: EXEC_MIMETYPE_LIST)
        if (type.inherits(a))
            return true;
    return false;
}

static void findExecutables(QStandardItemModel *m, const QDir& path) {
    auto entries = path.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    foreach(QFileInfo info, entries) {
        if (info.isDir()) {
            findExecutables(m, QDir(info.absoluteFilePath()));
        } else {
            QFile f(info.absoluteFilePath());
            QMimeType type = mimeDb.mimeTypeForFileNameAndData(info.absoluteFilePath(), &f);
            if (isExecMimetype(type)) {
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
    QDialog dialog(window());
    QListView *view = new QListView(&dialog);
    QHBoxLayout *layout = new QHBoxLayout(&dialog);
    layout->addWidget(view);
    QStandardItemModel *model = new QStandardItemModel(view);
    connect(view, &QListView::clicked, [view, model, &dialog](const QModelIndex& idx) {
        Q_UNUSED(idx);
        dialog.accept();
    });
    findExecutables(model, QDir(m_info.workingDir));
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
