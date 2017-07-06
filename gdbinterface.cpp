#include "gdbinterface.h"
#include "qtdesigner-gdb/breakhandler.h"

#include <QFileInfo>
#include <QtDebug>

#define MICMD(s) QString(s)

GdbInterface::GdbInterface(QObject *parent) : QObject(parent)
{
    gdbp = new QProcess(this);
    gdbp->setObjectName("gdbp");
    connect(gdbp, &QProcess::readyReadStandardError, this, &GdbInterface::gdbpReadFromStderr);
    connect(gdbp, &QProcess::readyReadStandardOutput, this, &GdbInterface::gdbpReadFromStdout);
    connect(gdbp, &QProcess::stateChanged, this, &GdbInterface::gdbpStateChanged);
    connect(gdbp, &QProcess::errorOccurred, this, &GdbInterface::gdbpErrorOccurred);
    static struct gdbwire_callbacks cb = {
        this,
        [](void *context, struct gdbwire_mi_stream_record *stream_record)
            { reinterpret_cast<GdbInterface*>(context)->callback_gdbwire_stream_record(stream_record); },
        [](void *context, struct gdbwire_mi_async_record *async_record)
            { reinterpret_cast<GdbInterface*>(context)->callback_gdbwire_async_record(async_record);},
        [](void *context, struct gdbwire_mi_result_record *result_record)
            { reinterpret_cast<GdbInterface*>(context)->callback_gdbwire_result_record(result_record); },
        [](void *context, const char *prompt) {
            reinterpret_cast<GdbInterface*>(context)->callback_gdbwire_prompt(prompt); },
        [](void *context, const char *mi, const char *token, struct gdbwire_mi_position position) {
            reinterpret_cast<GdbInterface*>(context)->callback_gdbwire_parse_error(mi, token, position); }
    };
    wire = gdbwire_create(cb);
}

GdbInterface::~GdbInterface()
{
    gdbwire_destroy(wire);
}

void GdbInterface::gdbpErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    emit outputLog(gdbp->errorString());
}

void GdbInterface::gdbpStateChanged(QProcess::ProcessState state)
{

}

void GdbInterface::gdbpReadFromStderr()
{
    emit outputLog(gdbp->readAllStandardError());
}

void GdbInterface::gdbpReadFromStdout()
{
    QByteArray out = gdbp->readAllStandardOutput();
    gdbwire_push_data(wire, out.constData(), static_cast<size_t>(out.size()));
}

void GdbInterface::start(const QString &gdbPath)
{
    gdbp->start(gdbPath, QStringList{ "-i", "mi" });
}

void GdbInterface::targetRemote(const QString &host, int port)
{
    sendToGdb(MICMD("-target-select remote %1 %2")
              .arg(host)
              .arg(port));
}

void GdbInterface::threadList()
{
    sendToGdb(MICMD("-thread-info"));
}

void GdbInterface::threadSelect(int n)
{
    sendToGdb(MICMD("-thread-select %1").arg(n));
}

void GdbInterface::breakInsert(const QString &location)
{
    sendToGdb(MICMD("-break-insert %1").arg(location));
}

void GdbInterface::breakDisable(int n)
{
    sendToGdb(MICMD("-break-disable %1").arg(n));
}

void GdbInterface::breakEnable(int n)
{
    sendToGdb(MICMD("-break-enable %1").arg(n));
}

void GdbInterface::commandRun()
{
    sendToGdb(MICMD("-exec-run"));
}

void GdbInterface::commandContinue()
{
    sendToGdb(MICMD("-exec-continue"));
}

void GdbInterface::commandFile(const QString& filename)
{
    sendToGdb(MICMD("-file-exec-and-symbols %1").arg(filename));
}

void GdbInterface::commandLoad()
{
    sendToGdb(MICMD("-target-download"));
}

void GdbInterface::commandNext()
{
    sendToGdb(MICMD("-exec-next"));
}

void GdbInterface::commandNextI()
{
    sendToGdb(MICMD("-exec-next-instruction"));
}

void GdbInterface::commandStep()
{
    sendToGdb(MICMD("-exec-step"));
}

void GdbInterface::commandStepI()
{
    sendToGdb(MICMD("-exec-step-instruction"));
}

void GdbInterface::commandFinish()
{
    sendToGdb(MICMD("-exec-finish"));
}

void GdbInterface::sendToGdb(const QString &data)
{
    gdbp->write(data.toLatin1());
#ifdef Q_OS_WIN
    gdbp->write("\r\n");
#else
    gdbp->write("\n");
#endif
}

void GdbInterface::callback_gdbwire_stream_record(gdbwire_mi_stream_record *stream_record)
{
    switch(stream_record->kind) {
    case GDBWIRE_MI_CONSOLE:
        emit outputConsole(QString(stream_record->cstring));
        break;
    case GDBWIRE_MI_LOG:
        emit outputLog(QString(stream_record->cstring));
        break;
    case GDBWIRE_MI_TARGET:
        emit outputGdb(QString(stream_record->cstring));
        break;
    }
}

void GdbInterface::callback_gdbwire_async_record(gdbwire_mi_async_record *async_record)
{
}

void GdbInterface::callback_gdbwire_result_record(gdbwire_mi_result_record *result_record)
{

}

void GdbInterface::callback_gdbwire_prompt(const char *prompt)
{

}

void GdbInterface::callback_gdbwire_parse_error(const char *mi, const char *token, gdbwire_mi_position position)
{

}
