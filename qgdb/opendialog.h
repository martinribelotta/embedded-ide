#ifndef FILE__ABOUTDIALOG_H
#define FILE__ABOUTDIALOG_H

#include <QDialog>

#include "settings.h"
#include "core.h"

namespace Ui {
class OpenDialog;
}

class OpenDialog : public QDialog {
	Q_OBJECT
public:

    explicit OpenDialog(QWidget *parent);
    virtual ~OpenDialog();

	void setProgram(QString program);
	void setArguments(QString program);
	QString getProgram();
	QString getArguments();

	void setTcpRemoteHost(QString host);
	QString getTcpRemoteHost();

	void setTcpRemotePort(int port);
	int getTcpRemotePort();


	void setTcpRemoteProgram(QString path);
	QString getTcpRemoteProgram();

	void setMode(ConnectionMode mode);
	ConnectionMode getMode();

	void setInitCommands(QStringList commandList);
	QStringList getInitCommands();

	void setGdbPath(QString path);
	QString getGdbPath();

	void loadConfig(Settings &cfg);
	void saveConfig(Settings *cfg);
    void updateExecList(const QString &path);

private:
	void onBrowseForProgram(QString *path);

private slots:
	void onConnectionTypeLocal(bool checked);
	void onConnectionTypeTcp(bool checked);

	void onSelectTcpProgram();
	void onSelectProgram();

    void on_execList_activated(const QModelIndex &index);

private:
    Ui::OpenDialog *ui;
};

#endif // FILE__ABOUTDIALOG_H
