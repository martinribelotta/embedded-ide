#include "debugui.h"
#include "ui_debugui.h"

#include "gdbstartdialog.h"
#include "loggerwidget.h"
#include "documentarea.h"
#include "projectview.h"
#include "gdbdebugger/gdbdebugger.h"

#include <QtDebug>

DebugUI::DebugUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugUI)
{
    ui->setupUi(this);
    iface = new GdbDebugger(this);
    connect(iface, &GdbDebugger::debugStarted, this, &DebugUI::debugStarted);
    connect(iface, &GdbDebugger::debugStoped, this, &DebugUI::debugStoped);

    setEnabled(false);
    connect(iface, &GdbDebugger::debugStarted, [this]() { setEnabled(true); });
    connect(iface, &GdbDebugger::debugStoped, [this]() { setEnabled(false); });

    ui->viewVars->setModel(iface->debugModel(VARS_MODEL));
    ui->viewWatch->setModel(iface->debugModel(WATCHES_MODEL));
    ui->viewStack->setModel(iface->debugModel(CALLSTACK_MODEL));

    connect(iface, &GdbDebugger::setCurrentLine, [this](const QString& file, int line) {
        docs->fileOpenAndSetIP(file, line, &view->makeInfo());
    });

    connect(iface, &GdbDebugger::debugLog, [this](DEBUG_LOG_TYPE type, const QString &log){
        qDebug() << log;
        switch(type) {
        case DebugConsoleLog:
            gdbLog->addText(log + '\n', Qt::blue);
            break;
        case DebugApplationLog:
            appLog->addText(log + '\n', Qt::blue);
            break;
        case DebugRuntimeLog:
            gdbLog->addText(log + '\n', Qt::red);
            break;
        case DebugErrorLog:
            gdbLog->addText(log + '\n', Qt::green);
            break;
        }
    });
}

DebugUI::~DebugUI()
{
    delete ui;
}

void DebugUI::startDebug(const QString &projectPath)
{
    GDBStartDialog d(projectPath, window());
    if (d.exec() == QDialog::Accepted) {
        auto cfg = d.config();
        QStringList argv;
        for(auto a: cfg.initCommands) {
            if (!a.isEmpty())
               argv << "-ex" << a;
        }
        iface->setWorkingDirectory(projectPath);
        iface->start(cfg.gdbProgram, argv, cfg.dbgProgram);
    }
}

void DebugUI::setLoggers(LoggerWidget *gdbLog, LoggerWidget *appLog)
{
    this->gdbLog=gdbLog;
    this->appLog=appLog;
}

void DebugUI::stopDebug()
{
    iface->stop();
}

void DebugUI::on_debugRun_clicked()
{
    iface->continueRun();
}

void DebugUI::on_debugNext_clicked()
{
    iface->stepOver();
}

void DebugUI::on_debugNextInto_clicked()
{
    iface->stepInto();
}

void DebugUI::on_debugNextToEnd_clicked()
{
    iface->stepOut();
}
