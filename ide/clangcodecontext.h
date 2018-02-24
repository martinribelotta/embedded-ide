#ifndef CLANGCODECONTEXT_H
#define CLANGCODECONTEXT_H

#include <QObject>
#include <QStringList>

class CodeEditor;
class QProcess;

class CLangCodeContext : public QObject
{
    Q_OBJECT
public:
    static const QStringList HANDLE_TYPE;

    explicit CLangCodeContext(CodeEditor *parent = 0);
    ~CLangCodeContext() = default;

    void setWorkingDir(const QString& path);

signals:
    void completionListUpdate(const QStringList& list);
    void discoverCompleted(const QStringList& incl, const QStringList& defs);

public slots:
    void startContextUpdate();
    void discoverFor(const QString &path);

private slots:
    void clangStarted();
    void clangTerminated();

private:
    QProcess *clangProc;
    CodeEditor *ed;
    QStringList includes;
    QStringList defines;
};

#endif // CLANGCODECONTEXT_H
