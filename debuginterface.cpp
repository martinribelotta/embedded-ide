#include "debuginterface.h"
#include "ui_debuginterface.h"

#include "qgdb/opendialog.h"

#include <QMimeDatabase>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>

struct DebugInterface::Priv_t {
    QList<StackFrameEntry> currentStackFrameList;
    TargetState currentTargetState;

    void fillStack() {
        Core::getInstance().getStackFrames();
    }
};

DebugInterface::DebugInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugInterface),
    priv(new DebugInterface::Priv_t),
    projectView(0l),
    documentArea(0l)
{
    ui->setupUi(this);
    priv->currentTargetState = TARGET_STOPPED;
    Core::getInstance().setListener(this);
}

DebugInterface::~DebugInterface()
{
    delete priv;
    delete ui;
}

void DebugInterface::ICore_onStopped(StopReason reason, QString path, int lineNo)
{
    qDebug() << "onStop" << reason << path << lineNo;
    priv->fillStack();
}

void DebugInterface::ICore_onStateChanged(TargetState state)
{
    qDebug() << "STATE:" << state;
    priv->currentTargetState = state;
    bool isStopped = state == TARGET_STOPPED;
    ui->buttonStopDebug->setEnabled(isStopped);
    ui->buttonDebugRun->setEnabled(isStopped);
    ui->buttonDebugContinue->setEnabled(isStopped);
    ui->buttonDebugStepInto->setEnabled(isStopped);
    ui->buttonDebugStepOut->setEnabled(isStopped);
    ui->buttonDebugStepOver->setEnabled(isStopped);
    ui->buttonStartDebug->setEnabled(!isStopped);
}

void DebugInterface::ICore_onSignalReceived(QString signalName)
{
    QMessageBox::information(this->window(), tr("Signal recevied"), tr("Signal <<%1>>").arg(signalName));
}

void DebugInterface::ICore_onLocalVarReset()
{
}

void DebugInterface::ICore_onLocalVarChanged(QString name, QString value)
{
}

void DebugInterface::ICore_onFrameVarReset()
{
}

void DebugInterface::ICore_onFrameVarChanged(QString name, QString value)
{
    // qDebug() << "var change" << name << value;
}

void DebugInterface::ICore_onWatchVarChanged(int watchId, QString name, QString value)
{
}

void DebugInterface::ICore_onConsoleStream(QString text)
{
    emit gdbOutput(text);
}

void DebugInterface::ICore_onBreakpointsChanged()
{
}

void DebugInterface::ICore_onThreadListChanged()
{
    QStandardItemModel *m = new QStandardItemModel(this);
    foreach(ThreadInfo thd, Core::getInstance().getThreadList()) {
        QStandardItem* thdItem = new QStandardItem(tr("[%3] %1: %2")
                                                   .arg(thd.m_name)
                                                   .arg(thd.m_func)
                                                   .arg(thd.id));
        thdItem->setData(QVariant::fromValue(thd), Qt::UserRole);
        qDebug() << "create thread" << thd.id;
        m->appendRow(thdItem);
    }
    ui->treeFrame->setModel(m);
}

void DebugInterface::ICore_onCurrentThreadChanged(int threadId)
{
    qDebug() << "thread change" << threadId;
    auto *m = qobject_cast<QStandardItemModel*>(ui->treeFrame->model());
    if (m && threadId > 0 && threadId <= m->invisibleRootItem()->rowCount()) {
        Core::getInstance().selectThread(threadId);
        QStandardItem *thdItem = m->invisibleRootItem()->child(threadId-1, 0);
        if (thdItem) {
            ThreadInfo info = thdItem->data(Qt::UserRole).value<ThreadInfo>();
            qDebug() << "select thread id" << info.id << info.m_func << info.m_name;
            ui->treeFrame->selectionModel()->select(thdItem->index(),
                                                    QItemSelectionModel::Rows |
                                                    QItemSelectionModel::Select);
        } else
            qDebug() << "No thread item for " << threadId;
    } else
        qDebug() << "ignoring it" << threadId;
}

void DebugInterface::ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList)
{
    qDebug() << "set stack" << stackFrameList.size();
    priv->currentStackFrameList = stackFrameList;
}

void DebugInterface::ICore_onMessage(QString message)
{
    emit gdbMessage(message);
}

void DebugInterface::ICore_onTargetOutput(QString message)
{
    emit applicationOutput(message);
}

void DebugInterface::ICore_onCurrentFrameChanged(int frameIdx)
{
    qDebug() << "current frame" << frameIdx << priv->currentStackFrameList.size();
    if (frameIdx >= 0 && frameIdx < priv->currentStackFrameList.size()) {
        StackFrameEntry &entry = priv->currentStackFrameList[priv->currentStackFrameList.size()-frameIdx-1];
        documentArea->fileOpenAndSetIP(entry.m_sourcePath, entry.m_line, &projectView->makeInfo());
        auto *m = qobject_cast<QStandardItemModel*>(ui->treeFrame->model());
        if (m) {
            QStandardItem *selectThread = nullptr;
            foreach(auto i, ui->treeFrame->selectionModel()->selectedIndexes()) {
                selectThread = m->itemFromIndex(i);
                if (selectThread) {
                    if (selectThread->data(Qt::UserRole).canConvert<ThreadInfo>()) {
                        break;
                    }
                }
            }
            if (selectThread) {
                ThreadInfo info = selectThread->data(Qt::UserRole).value<ThreadInfo>();
                qDebug() << "inserting frames on" << info.id << info.m_name;

            }
        }
    }
}

void DebugInterface::on_buttonStartDebug_clicked()
{
    if (!projectView)
        return;
    OpenDialog d(window());
    d.updateExecList(projectView->projectPath().absolutePath());
    Settings cfg;
    cfg.load(":/qgdb/gdb_default_profile.ini");
    d.loadConfig(cfg);
    if (d.exec() == QDialog::Accepted) {
        d.saveConfig(&cfg);
        if (cfg.m_connectionMode == MODE_LOCAL)
            Core::getInstance().initLocal(&cfg, cfg.m_gdbPath, cfg.m_lastProgram, cfg.m_argumentList);
        else
            Core::getInstance().initRemote(&cfg, cfg.m_gdbPath, cfg.m_tcpProgram, cfg.m_tcpHost, cfg.m_tcpPort);
        Core::getInstance().gdbSetBreakpointAtFunc("main");
        Core::getInstance().getSourceFiles();
    }
}

void DebugInterface::on_buttonStopDebug_clicked()
{
    Core::getInstance().stop();
}

void DebugInterface::on_buttonDebugRun_clicked()
{
    Core::getInstance().gdbRun();
}

void DebugInterface::on_buttonDebugStepOver_clicked()
{
    Core::getInstance().gdbNext();
}

void DebugInterface::on_buttonDebugStepInto_clicked()
{
    Core::getInstance().gdbStepIn();
}

void DebugInterface::on_buttonDebugStepOut_clicked()
{
    Core::getInstance().gdbStepOut();
}

void DebugInterface::on_buttonWatchAdd_clicked()
{

}

void DebugInterface::on_buttonWatchRemove_clicked()
{

}

void DebugInterface::on_buttonWatchClear_clicked()
{

}

void DebugInterface::on_buttonBreakRemove_clicked()
{

}

void DebugInterface::on_buttonBreakClear_clicked()
{

}

void DebugInterface::on_listBreakpoints_activated(const QModelIndex &index)
{

}

void DebugInterface::on_buttonDebugContinue_clicked()
{
    Core::getInstance().gdbContinue();
}
