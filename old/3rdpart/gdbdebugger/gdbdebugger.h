/**************************************************************************
** This file is part of LiteIDE
**
** Copyright (c) 2011-2016 LiteIDE Team. All rights reserved.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** In addition, as a special exception,  that plugins developed for LiteIDE,
** are allowed to remain closed sourced and can be distributed under any license .
** These rights are included in the file LGPL_EXCEPTION.txt in this package.
**
**************************************************************************/
// Module: gdbdebugger.h
// Creator: visualfc <visualfc@gmail.com>

#ifndef GDBDEBUGGER_H
#define GDBDEBUGGER_H

#include <gdbmi.h>

#include <QSet>
#include <QAbstractItemModel>
#include <QProcess>

enum DEBUG_MODEL_TYPE{
    ASYNC_MODEL = 1,
    VARS_MODEL,
    WATCHES_MODEL,
    CALLSTACK_MODEL,
    BREAKPOINTS_MODEL,
    THREADS_MODEL,
    LIBRARY_MODEL
};

enum DEBUG_LOG_TYPE {
    DebugConsoleLog = 1,
    DebugApplationLog,
    DebugRuntimeLog,
    DebugErrorLog
};

class QProcess;
class GdbHandleState
{
public:
    GdbHandleState() : m_exited(false),m_stopped(false) {}
    void clear()
    {
        m_reason.clear();
        m_exited = false;
        m_stopped = false;
    }
    void setExited(bool b) {m_exited = b;}
    void setStopped(bool b) {m_stopped = b;}
    void setReason(const QByteArray &reason) { m_reason = reason; }
    bool exited() const { return m_exited; }
    bool stopped() const { return m_stopped; }
    QByteArray reason() const { return m_reason; }
public:
    bool       m_exited;
    bool       m_stopped;
    QByteArray m_reason;
};

class GdbCmd
{
public:
    GdbCmd()
    {
    }
    GdbCmd(const QString &cmd)
    {
        setCmd(cmd);
    }
    GdbCmd(const QStringList &cmd)
    {
        setCmd(cmd);
    }
    void setCmd(const QString &cmd)
    {
        m_cmd = cmd;
        m_cookie.insert("cmd",m_cmd);
        m_cookie.insert("cmdList",cmd.split(" ",QString::SkipEmptyParts));
    }
    void setCmd(const QStringList &cmd)
    {
        m_cmd = cmd.join(" ");
        m_cookie.insert("cmd",m_cmd);
        m_cookie.insert("cmdList",cmd);
    }
    void insert(const QString &key, const QVariant &value)
    {
        m_cookie.insert(key,value);
    }
    QByteArray makeCmd(int index) const
    {
#ifdef Q_OS_WIN
        return QString("%1%2").arg(index,8,10,QLatin1Char('0')).arg(m_cmd).toLatin1();
#else
        return QString("%1%2").arg(index,8,10,QLatin1Char('0')).arg(m_cmd).toUtf8();
#endif
    }
    QMap<QString,QVariant> cookie() const
    {
        return m_cookie;
    }
protected:
    QString m_cmd;
    QMap<QString,QVariant> m_cookie;
};

class QStandardItemModel;
class QStandardItem;

class GdbDebugger : public QObject
{
    Q_OBJECT
private:
    GdbDebugger(QObject *parent = 0);
    ~GdbDebugger();

public:
    enum VarItemDataRole{
        VarNameRole = Qt::UserRole + 1,
        VarNumChildRole,
        VarExpanded
    };

    static GdbDebugger *instance();
public:
    virtual QString mimeType() const;
    virtual QAbstractItemModel *debugModel(DEBUG_MODEL_TYPE type);
    virtual void setWorkingDirectory(const QString &dir);
    virtual void setEnvironment (const QStringList &environment);
    virtual bool start(const QString &program, const QStringList &argv, const QString &progName);
    virtual void targetRemoteAttach(const QString& medium);
    virtual void stop();
    virtual bool isRunning();
    virtual void stepOver();
    virtual void stepInto();
    virtual void stepOut();
    virtual void continueRun();
    virtual void interruptRun();
    virtual void runToLine(const QString &fileName, int line);
    virtual void command(const QByteArray &cmd);
    virtual void enterAppText(const QString &text);
    virtual void enterDebugText(const QString &text);
    virtual void expandItem(QModelIndex index, DEBUG_MODEL_TYPE type);
    virtual void setInitBreakTable(const QMultiMap<QString,int> &bks);
    virtual void setInitWatchList(const QStringList &names);
    virtual void insertBreakPoint(const QString &fileName, int line);
    virtual void removeBreakPoint(const QString &fileName, int line);

public:
    virtual void command(const GdbCmd &cmd);
    virtual void createWatch(const QString &var);
    virtual void removeWatch(const QString &name);
    virtual void removeAllWatch();
    virtual void showFrame(QModelIndex index);
protected:
    void createWatchHelp(const QString &var, bool floating, bool watchModel);
    void removeWatchHelp(const QString &var, bool byName, bool children);
    void removeWatchByNameHelp(const QString &name, bool children);
    void command_helper(const GdbCmd &cmd, bool emitOut);
public slots:
    void appLoaded();
    void readStdError();
    void readStdOutput();
    void finished(int);
    void error(QProcess::ProcessError);
    void readTty(const QByteArray &data);

signals:
    void debugStarted();
    void debugStoped();
    void debugLoaded();
    void targetRemoteAttached();
    void debugLog(DEBUG_LOG_TYPE type, const QString &log);
    void setExpand(DEBUG_MODEL_TYPE type, const QModelIndex &index, bool expanded);
    void setCurrentLine(const QString &fileName, int line);
    void execStop(const QString& reason);
    void execRunning();
    void setFrameLine(const QString &fileName, int line);
    void watchCreated(const QString &watch,const QString &name);
    void watchRemoved(const QString &watch);
    void breakInserted(int id, const QString& file, int line);
    void breakDeleted(int id);

protected:
    void handleResponse(const QByteArray &buff);
    void handleStopped(const GdbMiValue &result);
    void handleLibrary(const GdbMiValue &result);
    void handleAsyncClass(const QByteArray &asyncClass, const GdbMiValue &result);
    void handleConsoleStream(const QByteArray &data);
    void handleTargetStream(const QByteArray &data);
    void handleLogStream(const QByteArray &data);
    void handleResultRecord(const GdbResponse &response);
    void handleResultStackListFrame(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleResultStackListVariables(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleResultVarCreate(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleResultVarListChildren(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleResultVarUpdate(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleResultVarDelete(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleResultVarUpdateValue(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleResultVarUpdateType(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleBreakInsert(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleBreakDelete(const GdbResponse &response, QMap<QString,QVariant> &map);
    void handleRemoteResponse(const GdbResponse &response, QMap<QString,QVariant> &map);

protected:
    void clear();
    void initGdb();
    void updateWatch();
    void updateLocals();
    void updateFrames();
    void updateBreaks();
    void updateVarTypeInfo(const QString &name);
    void updateVarListChildren(const QString &name);
    void updateVarValue(const QString &name);

protected:
    QProcess *m_process;
    QStandardItemModel *m_asyncModel;
    QStandardItemModel *m_varsModel;
    QStandardItemModel *m_watchModel;
    QStandardItemModel *m_framesModel;
    QStandardItemModel *m_libraryModel;
    QStandardItem   *m_asyncItem;
    QMap<int,QVariant> m_tokenCookieMap;
    QMap<QString,QString> m_varNameMap;
    QList<QString> m_watchList;
    QMap<QString,QStandardItem*> m_nameItemMap;
    QSet<QStandardItem*> m_varChangedItemList;
    QString m_gdbFilePath;
    QString m_runtimeFilePath;
    QByteArray m_inbuffer;
    GdbHandleState m_handleState;
    QMultiMap<QString,int>  m_initBks;
    QMap<QString,QString> m_locationBkMap;
    bool    m_busy;
    bool    m_gdbinit;
    bool    m_gdbexit;
    int     m_token;
};

#endif // GDBDEBUGGER_H
