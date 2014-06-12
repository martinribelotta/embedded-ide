#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <Qsci/qsciscintilla.h>
#include <QVariant>

class CodeEditor : public QsciScintilla
{
    Q_OBJECT
public:
    typedef QHash<QString, QString> Context_t;

    explicit CodeEditor(QWidget *parent = 0);
    virtual ~CodeEditor();

    static QFont defaultFont();

    Context_t context() const;
    QString fileName() const { return property("fileName").toString(); }
    void setFileName(const QString& name) { setProperty("fileName", name); }

signals:

public slots:

private:
    void initializeEditor();

private slots:
    void onTextChanged();
};

#endif // CODEEDITOR_H
