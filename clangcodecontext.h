#ifndef CLANGCODECONTEXT_H
#define CLANGCODECONTEXT_H

#include <QObject>

class CodeEditor;
class QProcess;

class CLangCodeContext : public QObject
{
    Q_OBJECT
public:
    explicit CLangCodeContext(CodeEditor *parent = 0);
    ~CLangCodeContext();

    void setWorkingDir(const QString& path);

signals:
    void completionListUpdate(const QStringList& list);

public slots:
    void startContextUpdate();

private slots:
    void clangStarted();
    void clangTerminated();

private:
    QProcess *clangProc;
    CodeEditor *ed;
};

#endif // CLANGCODECONTEXT_H
