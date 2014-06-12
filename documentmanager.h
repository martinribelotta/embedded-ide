#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QTabWidget>

class CodeEditor;

class DocumentManager : public QTabWidget
{
    Q_OBJECT
public:
    explicit DocumentManager(QWidget *parent = 0);

    CodeEditor *codeOf(int n) const;
    CodeEditor *currentCode() const;

signals:

public slots:
    CodeEditor *newEditor();
    void openEditor(const QString& fileName);
    bool saveCode(int idx);
    bool saveCode(CodeEditor *ed);
    void closeEditor(int idx);

private slots:
    void onTextChanged();

private:
    void handleChangeOf(CodeEditor *ed);
};

#endif // DOCUMENTMANAGER_H
