#ifndef DEBUGUI_H
#define DEBUGUI_H

#include <QWidget>

namespace Ui {
class DebugUI;
}

class DocumentArea;
class ProjectView;
class LoggerWidget;

class DebugUI : public QWidget
{
    Q_OBJECT

public:
    explicit DebugUI(QWidget *parent = 0);
    virtual ~DebugUI();

    void setDocumentArea(DocumentArea *docs) { this->docs = docs; }
    void setProjectView(ProjectView *view) { this->view = view; }
    void setLoggers(LoggerWidget *gdbLog, LoggerWidget *appLog);

    void startDebug();
    void stopDebug();

private slots:
    void on_debugRun_clicked();

    void on_debugNext_clicked();

    void on_debugNextInto_clicked();

    void on_debugNextToEnd_clicked();

private:
    Ui::DebugUI *ui;
    DocumentArea *docs;
    ProjectView *view;
    LoggerWidget *gdbLog;
    LoggerWidget *appLog;
};

#endif // DEBUGUI_H
