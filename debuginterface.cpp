#include "debuginterface.h"
#include "ui_debuginterface.h"

#include "qgdb/opendialog.h"

#include <QMimeDatabase>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>
#include <QKeyEvent>

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
    projectView(nullptr),
    documentArea(nullptr)
{
    ui->setupUi(this);
    priv->currentTargetState = TARGET_STOPPED;
    // Core::getInstance().setListener(this);

    ui->varWidget->setColumnCount(3);
    ui->varWidget->setColumnWidth(0, 80);
    QStringList names;
    names += "Name";
    names += "Value";
    names += "Type";
    ui->varWidget->setHeaderLabels(names);
    connect(ui->varWidget, SIGNAL(itemChanged(QTreeWidgetItem * ,int)), this, SLOT(onWatchWidgetCurrentItemChanged(QTreeWidgetItem * ,int)));
    connect(ui->varWidget, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this, SLOT(onWatchWidgetItemDoubleClicked(QTreeWidgetItem *, int )));

    ui->treeWidget_breakpoints->setColumnCount(2);
    ui->treeWidget_breakpoints->setColumnWidth(0, 80);
    names.clear();
    names += "Filename";
    names += "Func";
    names += "Line";
    names += "Addr";
    ui->treeWidget_breakpoints->setHeaderLabels(names);
    connect(ui->treeWidget_breakpoints, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this, SLOT(onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * ,int)));

    ui->autoWidget->setColumnCount(2);
    ui->autoWidget->setColumnWidth(0, 80);
    names.clear();
    names += "Name";
    names += "Value";
    ui->autoWidget->setHeaderLabels(names);
    connect(ui->autoWidget, SIGNAL(itemDoubleClicked ( QTreeWidgetItem * , int  )), this,
            SLOT(onAutoWidgetItemDoubleClicked(QTreeWidgetItem *, int )));

    QTreeWidget *treeWidget = ui->treeWidget_threads;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
    treeWidget->setColumnWidth(0, 200);

    connect(ui->treeWidget_threads, SIGNAL(itemSelectionChanged()), this,
            SLOT(onThreadWidgetSelectionChanged()));

    treeWidget = ui->treeWidget_stack;
    names.clear();
    names += "Name";
    treeWidget->setHeaderLabels(names);
    treeWidget->setColumnCount(1);
    treeWidget->setColumnWidth(0, 200);

    connect(ui->treeWidget_stack, SIGNAL(itemSelectionChanged()), this,
            SLOT(onStackWidgetSelectionChanged()));

    ui->splitter_3->setSizes(QList<int>{300, 120, 120});

    ui->buttonDebugStepOver->setDefaultAction(ui->actionNext);
    ui->buttonDebugStepInto->setDefaultAction(ui->actionStep_In);
    ui->buttonDebugStepOut->setDefaultAction(ui->actionStep_Out);
    ui->buttonDebugRun->setDefaultAction(ui->actionRun);
    ui->buttonDebugContinue->setDefaultAction(ui->actionContinue);
    connect(ui->actionStop, SIGNAL(triggered()), SLOT(onStop()));
    connect(ui->actionNext, SIGNAL(triggered()), SLOT(onNext()));
    connect(ui->actionStep_In, SIGNAL(triggered()), SLOT(onStepIn()));
    connect(ui->actionStep_Out, SIGNAL(triggered()), SLOT(onStepOut()));
    connect(ui->actionRun, SIGNAL(triggered()), SLOT(onRun()));
    connect(ui->actionContinue, SIGNAL(triggered()), SLOT(onContinue()));

    fillInVars();

    Core &core = Core::getInstance();
    core.setListener(this);

    installEventFilter(this);
}

DebugInterface::~DebugInterface()
{
    delete priv;
    delete ui;
}

bool DebugInterface::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        QWidget *widget = QApplication::focusWidget();

        // 'Delete' key pressed in the var widget
        if(widget == ui->varWidget && keyEvent->key() == Qt::Key_Delete) {
            QList<QTreeWidgetItem *> items = ui->varWidget->selectedItems();

            for(auto item : items) {
                // Delete the item
                Core &core = Core::getInstance();
                QTreeWidgetItem *rootItem = ui->varWidget->invisibleRootItem();
                int key = item->data(0, Qt::UserRole).toInt();
                if(key > 0) {
                    rootItem->removeChild(item);
                    core.gdbRemoveVarWatch(key);
                }
            }
        }
        //qDebug() << "key " << keyEvent->key() << " from " << obj << "focus " << widget;
    }
    return QObject::eventFilter(obj, event);
}

/**
 * @brief Execution has stopped.
 * @param lineNo   The line which is about to execute (1=first).
 */
void DebugInterface::ICore_onStopped(ICore::StopReason reason, QString path, int lineNo)
{
    Q_UNUSED(reason);

    if(reason == ICore::EXITED_NORMALLY) {
        QString title = "Program exited";
        QString text = "Program exited normally";
        documentArea->clearIp();
        QMessageBox::information (this, title, text);
        return;
    }

    m_currentLine = lineNo;
    m_currentFile = path;
    open(path);
    if(path.isEmpty()) {
        documentArea->clearIp();
    }
    fillInStack();
}

void DebugInterface::ICore_onLocalVarReset()
{
    QTreeWidget *autoWidget = ui->autoWidget;

    autoWidget->clear();
}

void DebugInterface::ICore_onLocalVarChanged(QString name, QString value)
{
    QString displayValue = value;
    QTreeWidget *autoWidget = ui->autoWidget;
    QTreeWidgetItem *item;
    QStringList names;

    //
    if(m_autoVarDispInfo.contains(name)) {
        DispInfo &dispInfo = m_autoVarDispInfo[name];
        dispInfo.orgValue = value;

        // Update the variable value
        if(dispInfo.orgFormat == DISP_DEC)
            displayValue = valueDisplay(value.toLongLong(), dispInfo.dispFormat);
    } else {
        DispInfo dispInfo;
        dispInfo.orgValue = value;
        dispInfo.orgFormat = findVarType(value);
        dispInfo.dispFormat = dispInfo.orgFormat;
        m_autoVarDispInfo[name] = dispInfo;
    }

    //
    names.clear();
    names += name;
    names += displayValue;
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    autoWidget->insertTopLevelItem(0, item);
}

void DebugInterface::ICore_onWatchVarChanged(int watchId, QString name, QString valueString)
{
    QTreeWidget *varWidget = ui->varWidget;
    QStringList names;

    Q_UNUSED(name);

    for(int i = 0;i < varWidget->topLevelItemCount();i++) {
        QTreeWidgetItem * item =  varWidget->topLevelItem(i);
        int itemKey = item->data(0, Qt::UserRole).toInt();
        //debugMsg("%s=%s", stringToCStr(name), stringToCStr(itemKey));
        if(watchId == itemKey) {
            if(m_watchVarDispInfo.contains(name)) {
                DispInfo &dispInfo = m_watchVarDispInfo[name];
                dispInfo.orgValue = valueString;

                // Update the variable value
                if(dispInfo.orgFormat == DISP_DEC) {
                    valueString = valueDisplay(valueString.toLongLong(), dispInfo.dispFormat);
                }
            }
            item->setText(1, valueString);
        }
    }
}

void DebugInterface::ICore_onConsoleStream(QString text)
{
    emit gdbOutput(text);
    // ui->logView->appendPlainText(text);
}

void DebugInterface::ICore_onMessage(QString message)
{
    emit gdbOutput(message);
    // ui->logView->appendPlainText(message);
}

void DebugInterface::fillInStack()
{
    Core &core = Core::getInstance();

    core.getStackFrames();
}

void DebugInterface::fillInVars()
{
    QTreeWidget *varWidget = ui->varWidget;
    QTreeWidgetItem *item;
    QStringList names;

    varWidget->clear();

    names.clear();
    names += "...";
    item = new QTreeWidgetItem(names);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    varWidget->insertTopLevelItem(0, item);
}


void DebugInterface::onThreadWidgetSelectionChanged( )
{
    // Get the new selected thread
    QTreeWidget *threadWidget = ui->treeWidget_threads;
    QList <QTreeWidgetItem *> selectedItems = threadWidget->selectedItems();
    if(selectedItems.size() > 0) {
        QTreeWidgetItem * currentItem;
        currentItem = selectedItems[0];
        int selectedThreadId = currentItem->data(0, Qt::UserRole).toInt();

        // Select the thread
        Core &core = Core::getInstance();
        core.selectThread(selectedThreadId);
    }
}

void DebugInterface::onStackWidgetSelectionChanged()
{
    Core &core = Core::getInstance();

    int selectedFrame = -1;
    // Get the new selected frame
    QTreeWidget *threadWidget = ui->treeWidget_stack;
    QList <QTreeWidgetItem *> selectedItems = threadWidget->selectedItems();
    if(selectedItems.size() > 0) {
        QTreeWidgetItem * currentItem;
        currentItem = selectedItems[0];
        selectedFrame = currentItem->data(0, Qt::UserRole).toInt();

        core.selectFrame(selectedFrame);
    }
}


DebugInterface::DispFormat DebugInterface::findVarType(QString dataString)
{
    if(dataString.indexOf("\"") != -1 ||
            dataString.indexOf("'") != -1 ||
            dataString.indexOf(".") != -1)
        return DISP_NATIVE;
    return DISP_DEC;
}


void DebugInterface::onWatchWidgetCurrentItemChanged( QTreeWidgetItem * current, int column)
{
    QTreeWidget *varWidget = ui->varWidget;
    Core &core = Core::getInstance();
    int oldKey = current->data(0, Qt::UserRole).toInt();
    QString oldName  = oldKey == -1 ? "" : core.gdbGetVarWatchName(oldKey);
    QString newName = current->text(0);

    if(column != 0)
        return;

    if(oldKey != -1 && oldName == newName)
        return;

    // debugMsg("oldName:'%s' newName:'%s' ", stringToCStr(oldName), stringToCStr(newName));

    if(newName == "...")
        newName = "";
    if(oldName == "...")
        oldName = "";

    // Nothing to do?
    if(oldName == "" && newName == "") {
        current->setText(0, "...");
        current->setText(1, "");
        current->setText(2, "");
    } else if(newName.isEmpty()) {
        // Remove a variable?
        QTreeWidgetItem *item = varWidget->invisibleRootItem();
        item->removeChild(current);

        core.gdbRemoveVarWatch(oldKey);

        m_watchVarDispInfo.remove(oldName);
    } else if(oldName == "") {
        // Add a new variable?
        // debugMsg("%s", stringToCStr(current->text(0)));
        QString value;
        int watchId;
        QString varType;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId) == 0) {
            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);

            DispInfo dispInfo;
            dispInfo.orgValue = value;
            dispInfo.orgFormat = findVarType(value);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[newName] = dispInfo;

            // Create a new dummy item
            QTreeWidgetItem *item;
            QStringList names;
            names += "...";
            item = new QTreeWidgetItem(names);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
            varWidget->addTopLevelItem(item);
        } else {
            current->setText(0, "...");
            current->setText(1, "");
            current->setText(2, "");
        }
    } else {
        // Change a existing variable
        //debugMsg("'%s' -> %s", stringToCStr(current->text(0)), stringToCStr(current->text(0)));

        // Remove old watch
        core.gdbRemoveVarWatch(oldKey);

        m_watchVarDispInfo.remove(oldName);

        QString value;
        int watchId;
        QString varType;
        if(core.gdbAddVarWatch(newName, &varType, &value, &watchId) == 0) {
            current->setData(0, Qt::UserRole, watchId);
            current->setText(1, value);
            current->setText(2, varType);

            // Add display information
            DispInfo dispInfo;
            dispInfo.orgValue = value;
            dispInfo.orgFormat = findVarType(value);
            dispInfo.dispFormat = dispInfo.orgFormat;
            m_watchVarDispInfo[newName] = dispInfo;

        } else {
            QTreeWidgetItem *rootItem = varWidget->invisibleRootItem();
            rootItem->removeChild(current);
        }
    }
}

/**
 * @brief Formats a string (Eg: 0x2) that represents a decimal value.
 */
QString DebugInterface::valueDisplay(long long val, DispFormat format)
{
    QString valueText;
    if(format == DISP_BIN) {
        QString subText;
        QString reverseText;
        do {
            subText.sprintf("%d", (int)(val & 0x1));
            reverseText = subText + reverseText;
            val = val>>1;
        } while(val > 0 || reverseText.length() % 8 != 0);

        for(int i = 0;i < reverseText.length();i++) {
            valueText += reverseText[i];
            if(i%4 == 3 && i+1 != reverseText.length())
                valueText += "_";
        }

        valueText = "0b" + valueText;

    } else if(format == DISP_HEX) {
        QString text;
        text.sprintf("%llx", val);

        // Prefix the string with suitable number of zeroes
        while(text.length()%4 != 0 && text.length() > 4)
            text = "0" + text;
        if(text.length()%2 != 0)
            text = "0" + text;

        for(int i = 0;i < text.length();i++) {
            valueText = valueText + text[i];
            if(i%4 == 3 && i+1 != text.length())
                valueText += "_";
        }
        valueText = "0x" + valueText;
    } else if(format == DISP_CHAR) {
        QChar c = QChar((int)val);
        if(c.isPrint())
            valueText.sprintf("'%c'", c.toLatin1());
        else
            valueText.sprintf("' '");
    } else if(format == DISP_DEC)
        valueText.sprintf("%lld", val);

    return valueText;
}


void DebugInterface::onWatchWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QTreeWidget *varWidget = ui->varWidget;


    if(column == 0)
        varWidget->editItem(item,column);
    else if(column == 1) {
        QString varName = item->text(0);
        if(m_watchVarDispInfo.contains(varName)) {
            DispInfo &dispInfo = m_watchVarDispInfo[varName];
            if(dispInfo.orgFormat == DISP_DEC) {
                long long val = dispInfo.orgValue.toLongLong();

                if(dispInfo.dispFormat == DISP_DEC)
                    dispInfo.dispFormat = DISP_HEX;
                else if(dispInfo.dispFormat == DISP_HEX)
                    dispInfo.dispFormat = DISP_BIN;
                else if(dispInfo.dispFormat == DISP_BIN)
                    dispInfo.dispFormat = DISP_CHAR;
                else if(dispInfo.dispFormat == DISP_CHAR)
                    dispInfo.dispFormat = DISP_DEC;

                QString valueText = valueDisplay(val, dispInfo.dispFormat);

                item->setText(1, valueText);
            }
        }
    }
}


void DebugInterface::onAutoWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QTreeWidget *varWidget = ui->varWidget;

    if(column == 0)
        varWidget->editItem(item,column);
    else if(column == 1) {
        QString varName = item->text(0);
        if(m_autoVarDispInfo.contains(varName)) {
            DispInfo &dispInfo = m_autoVarDispInfo[varName];
            if(dispInfo.orgFormat == DISP_DEC) {
                long long val = dispInfo.orgValue.toLongLong();

                if(dispInfo.dispFormat == DISP_DEC)
                    dispInfo.dispFormat = DISP_HEX;
                else if(dispInfo.dispFormat == DISP_HEX)
                    dispInfo.dispFormat = DISP_BIN;
                else if(dispInfo.dispFormat == DISP_BIN)
                    dispInfo.dispFormat = DISP_CHAR;
                else if(dispInfo.dispFormat == DISP_CHAR)
                    dispInfo.dispFormat = DISP_DEC;

                QString valueText = valueDisplay(val, dispInfo.dispFormat);

                item->setText(1, valueText);
            }
        }
    }
}

void DebugInterface::open(const QString& filename)
{
    if(filename.isEmpty())
        return;

    // qDebug() << __PRETTY_FUNCTION__;

    m_filename = filename;
    documentArea->fileOpenAndSetIP(m_filename, m_currentLine, &projectView->makeInfo());

    // Update the current line view
    ICore_onBreakpointsChanged();
}

void DebugInterface::onStop()
{
    Core &core = Core::getInstance();
    core.stop();
}

void DebugInterface::onNext()
{
    Core &core = Core::getInstance();
    core.gdbNext();
}

void DebugInterface::onRun()
{
    Core &core = Core::getInstance();
    core.gdbRun();
}

void DebugInterface::onContinue()
{
    Core &core = Core::getInstance();
    core.gdbContinue();
    documentArea->clearIp();
    // ui->codeView->setHighlightLine(-1);
}

void DebugInterface::onStepIn()
{
    Core &core = Core::getInstance();
    core.gdbStepIn();
}

void DebugInterface::onStepOut()
{
    Core &core = Core::getInstance();
    core.gdbStepOut();
}

void DebugInterface::ICore_onThreadListChanged()
{
    Core &core = Core::getInstance();

    QTreeWidget *threadWidget = ui->treeWidget_threads;
    threadWidget->clear();

    QList<ThreadInfo> list = core.getThreadList();

    for(auto & idx : list) {
        // Get name
        QString name = idx.m_name;

        // Add the item
        QStringList names;
        names.push_back(name);
        auto item = new QTreeWidgetItem(names);
        item->setData(0, Qt::UserRole, idx.id);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        threadWidget->insertTopLevelItem(0, item);
    }
}

void DebugInterface::ICore_onCurrentThreadChanged(int threadId)
{
    QTreeWidget *threadWidget = ui->treeWidget_threads;
    QTreeWidgetItem *rootItem = threadWidget->invisibleRootItem();

    for(int i = 0; i < rootItem->childCount(); i++) {
        QTreeWidgetItem *item = rootItem->child(i);

        int id = item->data(0, Qt::UserRole).toInt();
        if(id == threadId)
            item->setSelected(true);
        else
            item->setSelected(false);
    }
}

static QString longLongToHexString(unsigned long long num)
{
    QString newStr;
    QString str;
    str.sprintf("%llx", num);
    if(num != 0) {
        while(str.length()%4 != 0)
            str = "0" + str;

        for(int i = str.length()-1;i >= 0;i--) {
            newStr += str[str.length()-i-1];
            if(i%4 == 0 && i != 0)
                newStr += "_";
        }
        str = newStr;
    }

    return "0x" + str;
}

void DebugInterface::ICore_onBreakpointsChanged()
{
    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    QVector<int> numList;
    // Update the breakpoint list widget
    ui->treeWidget_breakpoints->clear();
    for(int i = 0;i <  bklist.size();i++) {
        BreakPoint* bk = bklist[i];

        QStringList nameList;
        QString name;
        nameList.append(QFileInfo(bk->fullname).fileName());
        nameList.append(bk->m_funcName);
        name.sprintf("%d", bk->lineNo);
        nameList.append(name);
        nameList.append(longLongToHexString(bk->m_addr));

        auto item = new QTreeWidgetItem(nameList);
        item->setData(0, Qt::UserRole, i);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Add the item to the widget
        ui->treeWidget_breakpoints->insertTopLevelItem(0, item);
    }

    // Update the fileview
    for(auto bk : bklist) {
        if(bk->fullname == m_filename)
            numList.push_back(bk->lineNo);
    }

    // ui->codeView->setBreakpoints(numList);
    // ui->codeView->update();
}

void DebugInterface::ICore_onStackFrameChange(QList<StackFrameEntry> stackFrameList)
{
    m_stackFrameList = stackFrameList;
    QTreeWidget *stackWidget = ui->treeWidget_stack;

    stackWidget->clear();

    for(int idx = 0;idx < stackFrameList.size();idx++) {
        // Get name
        StackFrameEntry &entry = stackFrameList[stackFrameList.size()-idx-1];

        // Create the item
        QStringList names;
        names.push_back(entry.m_functionName);

        auto item = new QTreeWidgetItem(names);

        item->setData(0, Qt::UserRole, idx);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        // Add the item to the widget
        stackWidget->insertTopLevelItem(0, item);
    }
}

/**
 * @brief The current frame has changed.
 * @param frameIdx    The frame  (0 being the newest frame)
 */
void DebugInterface::ICore_onCurrentFrameChanged(int frameIdx)
{
    QTreeWidget *threadWidget = ui->treeWidget_stack;
    QTreeWidgetItem *rootItem = threadWidget->invisibleRootItem();

    qDebug() << "frame change" << frameIdx;
    // Update the sourceview (with the current row).
    if(frameIdx < m_stackFrameList.size()) {
        StackFrameEntry &entry = m_stackFrameList[m_stackFrameList.size()-frameIdx-1];

        m_currentLine = entry.m_line;
        m_currentFile = entry.m_sourcePath;
        open(m_currentFile);
        qDebug() << "frame entry" << entry.m_line << entry.m_functionName << entry.m_sourcePath;
        //ui->codeView->setHighlightLine(-1);
    }

    for(int i = 0;i < rootItem->childCount();i++) {
        QTreeWidgetItem *item = rootItem->child(i);

        int id = item->data(0, Qt::UserRole).toInt();
        if(id == frameIdx)
            item->setSelected(true);
        else
            item->setSelected(false);
    }
}

void DebugInterface::ICore_onFrameVarReset()
{
}

void DebugInterface::ICore_onFrameVarChanged(QString name, QString value)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
}

void DebugInterface::onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * item,int column)
{
    Q_UNUSED(column);

    Core &core = Core::getInstance();
    QList<BreakPoint*>  bklist = core.getBreakPoints();
    int idx = item->data(0, Qt::UserRole).toInt();
    BreakPoint* bk = bklist[idx];

    documentArea->fileOpenAndSetIP(bk->fullname, bk->lineNo, &projectView->makeInfo());
    //ensureLineIsVisible(bk->lineNo);
}

void DebugInterface::ICore_onSignalReceived(QString signalName)
{
    if(signalName != "SIGINT") {
        QString msgText = tr("Program received signal %1.").arg(signalName);
        QString title = tr("Signal received");
        QMessageBox::warning(this, title, msgText);
    }

    //XXX
    //ui->codeView->disableCurrentLine();

    fillInStack();
}

void DebugInterface::ICore_onTargetOutput(QString message)
{
    if(message.endsWith("\n"))
        message = message.left(message.length()-1);
    if(message.endsWith("\r"))
        message = message.left(message.length()-1);
    emit applicationOutput(message);
    // ui->targetOutputView->appendPlainText(message);
}

void DebugInterface::ICore_onStateChanged(TargetState state)
{
    ui->actionNext->setEnabled(state == TARGET_STOPPED ? true : false);
    ui->actionStep_In->setEnabled(state == TARGET_STOPPED ? true : false);
    ui->actionStep_Out->setEnabled(state == TARGET_STOPPED ? true : false);
    ui->actionStop->setEnabled(state == TARGET_STOPPED ? false : true);
    ui->actionContinue->setEnabled(state == TARGET_STOPPED ? true : false);
    ui->actionRun->setEnabled(state == TARGET_STOPPED ? true : false);

    if(state == TARGET_RUNNING) {
        ui->treeWidget_stack->clear();
        ui->autoWidget->clear();
    }
}

void DebugInterface::on_buttonStartStopDebug_clicked()
{
    if (!projectView)
        return;
    if (ui->buttonStartStopDebug->isChecked()) {
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
        } else
            ui->buttonStartStopDebug->setChecked(false);
    } else {
        onStop();
    }
}
