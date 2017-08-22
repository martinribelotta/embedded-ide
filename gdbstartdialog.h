#ifndef GDBSTARTDIALOG_H
#define GDBSTARTDIALOG_H

#include <QDialog>

namespace Ui {
class GDBStartDialog;
}

class GDBStartDialog : public QDialog
{
    Q_OBJECT

public:
    struct GdbConfig {
        QString gdbProgram;
        QString dbgProgram;
        QStringList initCommands;
    };

    explicit GDBStartDialog(const QString& currentProjectPath, QWidget *parent = 0);
    ~GDBStartDialog();

    GdbConfig config() const;

private slots:
    void on_buttonLoadPredefinedConfig_clicked();

    void on_buttonLoadGDBExecutable_clicked();

    void on_buttonLoadProgramExecutable_clicked();

    void on_buttonFindProgramExecutable_clicked();

private:
    Ui::GDBStartDialog *ui;
    QString m_currentProjectPath;
};

#endif // GDBSTARTDIALOG_H
