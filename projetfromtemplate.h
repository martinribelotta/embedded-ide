#ifndef PROJETFROMTEMPLATE_H
#define PROJETFROMTEMPLATE_H

#include <QObject>
#include <QProcess>

class ProjetFromTemplate : public QObject
{
    Q_OBJECT
public:
    explicit ProjetFromTemplate(const QString& projectName, const QString& templateText,
                                QObject *parent, const char *slotName);

signals:
    void endOfCreation(const QString& project, const QString& error);

public slots:
    void start();

private slots:
    void onStarted();
    void onFinish(int ret, QProcess::ExitStatus status);
    void onError(QProcess::ProcessError err);

private:
    QProcess *proc;
    QString m_project;
    QString m_templateText;
};

#endif // PROJETFROMTEMPLATE_H
