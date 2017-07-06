#ifndef GDBINTERFACE_H
#define GDBINTERFACE_H

#include <QObject>
#include <QList>
#include <QProcess>

#include <functional>

#include "qtdesigner-gdb/gdbmi.h"
#include "qtdesigner-gdb/breakpoint.h"
#include "qtdesigner-gdb/elfreader.h"

#include "gdbwire.h"

//////////////////////////////////////////////////////////////////
//
// Module
//
//////////////////////////////////////////////////////////////////

class Module
{
public:
    Module() : symbolsRead(UnknownReadState) {}

public:
    enum SymbolReadState {
        UnknownReadState,  // Not tried.
        ReadFailed,        // Tried to read, but failed.
        ReadOk             // Dwarf index available.
    };
    QString moduleName;
    QString modulePath;
    QString hostPath;
    SymbolReadState symbolsRead;
    quint64 startAddress;
    quint64 endAddress;

    Utils::ElfData elfData;
};

//////////////////////////////////////////////////////////////////
//
// Symbol
//
//////////////////////////////////////////////////////////////////

class Symbol
{
public:
    QString address;
    QString state;
    QString name;
    QString section;
    QString demangled;
};

typedef QVector<Symbol> Symbols;

typedef QVector<Module> Modules;

class GdbInterface : public QObject
{
    Q_OBJECT
public:
    struct Frame {
        quint64 addr;
        QString func;
        QString file;
        int line;
    };

    enum ProgramState {
        ProgramNotStarted,
        ProgramRunning,
        ProgramStopped,
        ProgramError
    };

    explicit GdbInterface(QObject *parent = 0);
    virtual ~GdbInterface();


private slots:
    void gdbpErrorOccurred(QProcess::ProcessError error);
    void gdbpStateChanged(QProcess::ProcessState state);
    void gdbpReadFromStderr();
    void gdbpReadFromStdout();

signals:
    void programStateChange(ProgramState st);
    void updateContext(const QList<Frame> &frames, int current);
    void outputGdb(const QString& text);
    void outputConsole(const QString& text);
    void outputLog(const QString& text);
    void breakInserted(const BreakpointResponse& bp);
    void breakEnabled(int n);
    void breakDisabled(int n);

public slots:
    void start(const QString& gdbPath);
    void targetRemote(const QString& host, int port);

    void threadList();
    void threadSelect(int n);

    void breakInsert(const QString& location);
    void breakDisable(int n);
    void breakEnable(int n);

    void commandRun();
    void commandContinue();
    void commandFile(const QString& filename);
    void commandLoad();
    void commandNext();
    void commandNextI();
    void commandStep();
    void commandStepI();
    void commandFinish();

private:
    QProcess *gdbp;
    struct gdbwire *wire;

    void sendToGdb(const QString& data);
    void callback_gdbwire_stream_record(struct gdbwire_mi_stream_record *stream_record);
    void callback_gdbwire_async_record(struct gdbwire_mi_async_record *async_record);
    void callback_gdbwire_result_record(struct gdbwire_mi_result_record *result_record);
    void callback_gdbwire_prompt(const char *prompt);
    void callback_gdbwire_parse_error(const char *mi, const char *token, struct gdbwire_mi_position position);
};

#endif // GDBINTERFACE_H
