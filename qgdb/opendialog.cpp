#include "opendialog.h"
#include "ui_opendialog.h"

#include "version.h"
#include "log.h"
#include "util.h"

#include <QFileDialog>
#include <QDir>
#include <QStringListModel>
#include <QMimeDatabase>
#include <QStandardItemModel>


OpenDialog::OpenDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
    connect(ui->pushButton_selectFile, SIGNAL(clicked()), SLOT(onSelectProgram()));
    connect(ui->pushButton_selectTcpProgram, SIGNAL(clicked()), SLOT(onSelectTcpProgram()));
    connect(ui->radioButton_localProgram, SIGNAL(toggled(bool)), SLOT(onConnectionTypeLocal(bool)));
    connect(ui->radioButton_gdbServerTcp, SIGNAL(toggled(bool)), SLOT(onConnectionTypeTcp(bool)));
}

OpenDialog::~OpenDialog()
{
    delete ui;
}

void OpenDialog::setMode(ConnectionMode mode)
{
    ui->radioButton_localProgram->setChecked(false);
    ui->radioButton_gdbServerTcp->setChecked(false);
	onConnectionTypeLocal(false);
	onConnectionTypeTcp(false);

	if(mode == MODE_TCP) {
        ui->radioButton_gdbServerTcp->setChecked(true);
		onConnectionTypeTcp(true);        
	} else { // if(mode == MODE_LOCAL)
        ui->radioButton_localProgram->setChecked(true);
		onConnectionTypeLocal(true);
	}
}

ConnectionMode OpenDialog::getMode()
{
    if(ui->radioButton_gdbServerTcp->isChecked())
		return MODE_TCP;
	else
		return MODE_LOCAL;
}

QString OpenDialog::getProgram()
{
    return ui->lineEdit_program->text();
}

QString OpenDialog::getArguments()
{
    return ui->lineEdit_arguments->text();
}

void OpenDialog::setProgram(QString program)
{
    ui->lineEdit_program->setText(program);
}

void OpenDialog::setInitCommands(QStringList commandList)
{
	QString str;
	str = commandList.join("\n");
    ui->plainTextEdit_initCommands->setPlainText(str);
}

QStringList OpenDialog::getInitCommands()
{
    return ui->plainTextEdit_initCommands->toPlainText().split("\n");
}    

void OpenDialog::setArguments(QString arguments)
{
    ui->lineEdit_arguments->setText(arguments);

}

void OpenDialog::onBrowseForProgram(QString *path)
{
	// Get start dir
	QString startPath = *path;
	if(!startPath.isEmpty())
		dividePath(startPath, NULL, &startPath);
	else
		startPath = QDir::currentPath();

	// Open dialog
	QString fileName = QFileDialog::getOpenFileName(this,
			tr("Select Program"), startPath, tr("All Files (*)"));
	if(!fileName.isEmpty())
        *path = fileName;
}


static QMimeDatabase mimeDb;

#ifdef Q_OS_UNIX
#define EXEC_MIMETYPE "application/x-executable"
#else
#define EXEC_MIMETYPE "application/x-msdownload"
#endif

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

void OpenDialog::updateExecList(const QString& path)
{
    if (ui->execList->model())
        delete ui->execList->model();
    auto m = new QStandardItemModel(this);
    findExecutables(m, QDir(path));
    ui->execList->setModel(m);
}

void OpenDialog::onSelectTcpProgram()
{
    QString path = ui->lineEdit_tcpProgram->text();

	onBrowseForProgram(&path);

	// Fill in the selected path
    ui->lineEdit_tcpProgram->setText(path);
}

void OpenDialog::onSelectProgram()
{
    QString path = ui->lineEdit_program->text();

	onBrowseForProgram(&path);

	// Fill in the selected path
    ui->lineEdit_program->setText(path);
}

void OpenDialog::onConnectionTypeLocal(bool checked)
{
    ui->lineEdit_program->setEnabled(checked);
    ui->pushButton_selectFile->setEnabled(checked);
    ui->lineEdit_arguments->setEnabled(checked);
}

void OpenDialog::onConnectionTypeTcp(bool checked)
{
    ui->pushButton_selectTcpProgram->setEnabled(checked);
    ui->lineEdit_tcpHost->setEnabled(checked);
    ui->lineEdit_tcpPort->setEnabled(checked);
    ui->lineEdit_tcpProgram->setEnabled(checked);
}

void OpenDialog::setTcpRemoteHost(QString host)
{
    ui->lineEdit_tcpHost->setText(host);
}

QString OpenDialog::getTcpRemoteHost()
{
    return ui->lineEdit_tcpHost->text();
}


void OpenDialog::setTcpRemoteProgram(QString path)
{
    ui->lineEdit_tcpProgram->setText(path);
}

QString OpenDialog::getTcpRemoteProgram()
{
    return ui->lineEdit_tcpProgram->text();
}


void OpenDialog::setTcpRemotePort(int port)
{
	QString portStr;
	portStr.sprintf("%d", port);
    ui->lineEdit_tcpPort->setText(portStr);
}

int OpenDialog::getTcpRemotePort()
{
    return ui->lineEdit_tcpPort->text().toInt();
}


QString OpenDialog::getGdbPath()
{
    return ui->lineEdit_gdbCommand->text();
}


void OpenDialog::setGdbPath(QString path)
{
    return ui->lineEdit_gdbCommand->setText(path);
}

void OpenDialog::saveConfig(Settings *cfg)
{
	OpenDialog &dlg = *this;
	cfg->m_lastProgram = dlg.getProgram();
	cfg->m_argumentList = dlg.getArguments().split(' ');
	cfg->m_connectionMode = dlg.getMode();
	cfg->m_tcpPort = dlg.getTcpRemotePort();
	cfg->m_tcpHost = dlg.getTcpRemoteHost();
	cfg->m_tcpProgram = dlg.getTcpRemoteProgram();
	cfg->m_initCommands = dlg.getInitCommands();
	cfg->m_gdbPath = dlg.getGdbPath();

    if(dlg.ui->checkBox_reloadBreakpoints->checkState() == Qt::Checked)
		cfg->m_reloadBreakpoints = true;
	else
		cfg->m_reloadBreakpoints = false;
}

void OpenDialog::loadConfig(Settings &cfg)
{
	OpenDialog &dlg = *this;
	dlg.setMode(cfg.m_connectionMode);

	dlg.setTcpRemotePort(cfg.m_tcpPort);
	dlg.setTcpRemoteHost(cfg.m_tcpHost);
	dlg.setTcpRemoteProgram(cfg.m_tcpProgram);
	dlg.setInitCommands(cfg.m_initCommands);
	dlg.setGdbPath(cfg.m_gdbPath);

    dlg.ui->checkBox_reloadBreakpoints->setChecked(cfg.m_reloadBreakpoints);

	dlg.setProgram(cfg.m_lastProgram);
	QStringList defList;
	dlg.setArguments(cfg.m_argumentList.join(" "));
}

void OpenDialog::on_execList_activated(const QModelIndex &index)
{
    auto *p = qobject_cast<QStandardItemModel*>(ui->execList->model());
    if (p) {
        auto item = p->itemFromIndex(index);
        QString path = item->data().toString();
        if (ui->radioButton_gdbServerTcp->isChecked()) {
            ui->lineEdit_tcpProgram->setText(path);
        } else {
            ui->lineEdit_program->setText(path);
        }
    }
}
