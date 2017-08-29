#include "debugui.h"
#include "ui_debugui.h"

#include "gdbstartdialog.h"
#include "loggerwidget.h"
#include "documentarea.h"
#include "projectview.h"
#include "gdbdebugger/gdbdebugger.h"

#include <QTimer>
#include <QtDebug>

DebugUI::DebugUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugUI)
{
    ui->setupUi(this);

    setEnabled(false);
    setProperty("isRunning", false);
    connect(GdbDebugger::instance(), &GdbDebugger::debugStarted, [this]() { setEnabled(true); });
    connect(GdbDebugger::instance(), &GdbDebugger::debugStoped, [this]() { setEnabled(false); });
    connect(GdbDebugger::instance(), &GdbDebugger::execStop, [this](const QString& r) {
        qDebug() << "stoped by" << r;
        setProperty("isRunning", false);
        ui->debugRun->setIcon(QIcon(":/images/actions/media-playback-start.svg"));
    });
    connect(GdbDebugger::instance(), &GdbDebugger::execRunning, [this]() {
        qDebug() << "exec run";
        setProperty("isRunning", true);
        ui->debugRun->setIcon(QIcon(":/images/actions/media-playback-pause.svg"));
    });

    ui->viewVars->setModel(GdbDebugger::instance()->debugModel(VARS_MODEL));
    ui->viewWatch->setModel(GdbDebugger::instance()->debugModel(WATCHES_MODEL));
    ui->viewStack->setModel(GdbDebugger::instance()->debugModel(CALLSTACK_MODEL));

    connect(GdbDebugger::instance(), &GdbDebugger::setCurrentLine, [this](const QString& file, int line) {
        docs->fileOpenAndSetIP(file, line, &view->makeInfo());
    });

    connect(GdbDebugger::instance(), &GdbDebugger::debugLog, [this](DEBUG_LOG_TYPE type, const QString &log){
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

void DebugUI::startDebug()
{
    GDBStartDialog d(view->makeInfo(), window());
    if (d.exec() == QDialog::Accepted) {
        auto cfg = d.config();
        QStringList argv;
        for(auto a: cfg.initCommands)
            if (!a.isEmpty())
               argv << "-ex" << a;
        auto executor = [cfg, argv, this]() {
            GdbDebugger::instance()->setWorkingDirectory(view->projectPath().absolutePath());
            GdbDebugger::instance()->start(cfg.gdbProgram, argv, cfg.dbgProgram);
        };
        if (cfg.premakeTarget.isEmpty()) {
            executor();
        } else {
            view->doTarget(cfg.premakeTarget);
            QTimer::singleShot(cfg.startupDelay, executor);
        }
    }
}

void DebugUI::setLoggers(LoggerWidget *gdbLog, LoggerWidget *appLog)
{
    this->gdbLog=gdbLog;
    this->appLog=appLog;
}

void DebugUI::stopDebug()
{
    GdbDebugger::instance()->stop();
}

void DebugUI::on_debugRun_clicked()
{
    bool isRunning = property("isRunning").toBool();
    qDebug() << __PRETTY_FUNCTION__ << "isRunning" << isRunning;
    if (isRunning) {
        GdbDebugger::instance()->interruptRun();
    } else {
        GdbDebugger::instance()->continueRun();
    }
}

void DebugUI::on_debugNext_clicked()
{
    GdbDebugger::instance()->stepOver();
}

void DebugUI::on_debugNextInto_clicked()
{
    GdbDebugger::instance()->stepInto();
}

void DebugUI::on_debugNextToEnd_clicked()
{
    GdbDebugger::instance()->stepOut();
}
