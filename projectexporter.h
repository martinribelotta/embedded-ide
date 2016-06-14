#ifndef PROJECTEXPORTER_H
#define PROJECTEXPORTER_H

#include <QObject>
#include <QProcess>
#include <QDir>
#include <QTemporaryDir>

class QTemporaryDir;

class ProjectExporter : public QObject
{
    Q_OBJECT
public:
    explicit ProjectExporter(const QString& exportFile,
                             const QString& projectPath,
                             QObject *parent,
                             const char *slotFinish);

signals:
    void end(const QString& error);

public slots:
    void start();

private slots:
    void on_proc_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void on_proc_error(QProcess::ProcessError error);

private:
    QString m_exportFile;
    QString m_projectPath;
    QProcess *proc;
    QTemporaryDir tmpDir;
};

#endif // PROJECTEXPORTER_H
