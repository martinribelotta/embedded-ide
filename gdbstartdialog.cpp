#include "gdbstartdialog.h"
#include "ui_gdbstartdialog.h"

#include <QFileDialog>
#include <QListView>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardItemModel>
#include <QStringListModel>

#include "appconfig.h"

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

static QStringList findExecutables(const QDir& path) {
    QStringList list;
    auto entries = path.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for(const auto& info: entries) {
        if (info.isDir()) {
            list += findExecutables(QDir(info.absoluteFilePath()));
        } else {
            QFile f(info.absoluteFilePath());
            QMimeType type = mimeDb.mimeTypeForFileNameAndData(info.absoluteFilePath(), &f);
            if (isExecMimetype(type)) {
                list << info.absoluteFilePath();
            }
        }
    }
    return list;
}

GDBStartDialog::GDBStartDialog(const MakefileInfo &info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GDBStartDialog),
    m_info(info)
{
    ui->setupUi(this);
    ui->textEditCommands->setFont(AppConfig::systemMonoFont());

    auto executableListModel = new QStringListModel(findExecutables(QDir(m_info.workingDir)), this);
    ui->comboProgramExecutable->setModel(executableListModel);
    ui->comboProgramExecutable->setCurrentIndex(0);

    QStringList gdbPrefixed;
    for(const auto& e: info.prefixs)
        gdbPrefixed << QString("%1gdb").arg(e);
    gdbPrefixed << "gdb";
    auto gdbExecListModel = new QStringListModel(gdbPrefixed, this);
    ui->comboGdbExecutable->setModel(gdbExecListModel);
    ui->comboGdbExecutable->setCurrentIndex(0);
}

GDBStartDialog::~GDBStartDialog()
{
    delete ui;
}

GDBStartDialog::GdbConfig GDBStartDialog::config() const
{
    return GdbConfig{
        ui->comboGdbExecutable->currentText(),
        ui->comboProgramExecutable->currentText(),
        ui->textEditCommands->toPlainText().split(QRegularExpression(R"(\r?\n)"))
    };
}

void GDBStartDialog::on_buttonLoadGDBExecutable_clicked()
{
    QString name = QFileDialog::getOpenFileName(window(), tr("Open GDB executable"),
                                                QDir::homePath(),
                                                tr("GDB Configuration files (*gdb*);;"
                                                   "All files (*)"));
    if (!name.isEmpty())
        ui->comboGdbExecutable->setCurrentText(name);
}

void GDBStartDialog::on_buttonLoadProgramExecutable_clicked()
{
    QString name = QFileDialog::getOpenFileName(window(), tr("Open executable to debug"),
                                                m_info.workingDir, tr(
#ifdef Q_OS_WIN
                                                    "Executables (*.exe);;"
#endif
                                                    "All files (*)"));
    if (!name.isEmpty()) {
        ui->comboProgramExecutable->setCurrentText(name);
    }
}
