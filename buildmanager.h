#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include <QObject>
#include <QTextBrowser>
#include <QProcess>

class BuildManager : public QObject
{
    Q_OBJECT
public:
    explicit BuildManager(QObject *parent = 0);
    virtual ~BuildManager();

    void setStdout(QTextBrowser *textBrowser);
    void setStderr(QTextBrowser *textBrowser);

signals:
    void finished(int status);
    void stdoutLine(const QString& line);
    void stderrLine(const QString& line);

public slots:
    void start(const QString& command);

private slots:
    void on_proc_readyReadStandardError();
    void on_proc_readyReadStandardOutput();
    void on_proc_finished(int exitCode, QProcess::ExitStatus status);

private:
    struct Priv_t;
    Priv_t *priv;
};

#endif // BUILDMANAGER_H
