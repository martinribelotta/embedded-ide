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

static QStringList buildPosibleGdbNames(const QStringList& prefixsList)
{
    QStringList gdbPrefixed;
    for(const auto& e: prefixsList)
        gdbPrefixed << QString("%1gdb").arg(e);
    gdbPrefixed << "gdb";
    return gdbPrefixed;
}

static const QStringList POSIBLE_PRETARGET_MAKE = {
    R"(.*debug\w*.*)",
    R"(.*openocd.*)",
};

static int findPosiblePreTargetMake(const QComboBox *c)
{
    for(int i=0; i<c->count(); i++)
        for(const auto& rt: POSIBLE_PRETARGET_MAKE)
            if (QRegularExpression(rt).match(c->itemText(i)).hasMatch())
                return i;
    return 0;
}

GDBStartDialog::GDBStartDialog(const MakefileInfo &info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GDBStartDialog),
    m_info(info)
{
    ui->setupUi(this);
    ui->textEditCommands->setFont(AppConfig::systemMonoFont());

    ui->comboProgramExecutable->addItems(findExecutables(QDir(m_info.workingDir)));
    ui->comboProgramExecutable->setCurrentIndex(0);

    ui->comboGdbExecutable->addItems(buildPosibleGdbNames(m_info.prefixs));
    ui->comboGdbExecutable->setCurrentIndex(0);

    ui->comboPreTargetMake->addItems(m_info.targets);
    ui->comboPreTargetMake->setCurrentIndex(findPosiblePreTargetMake(ui->comboPreTargetMake));
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
        ui->groupPreTarget->isChecked()?
                ui->comboPreTargetMake->currentText() : QString(),
        ui->textEditCommands->toPlainText().split(QRegularExpression(R"(\r?\n)")),
        ui->spinStartDelay->value()
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
