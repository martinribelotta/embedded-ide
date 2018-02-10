#ifndef CLANGDMANAGER_H
#define CLANGDMANAGER_H

#include <QObject>

class ClangdManager : public QObject
{
    Q_OBJECT
private:
    explicit ClangdManager(QObject *parent = nullptr);
    virtual ~ClangdManager();

public:
    static ClangdManager *instance();

signals:

public slots:
    void start(const QString& workspace);
    void addTarget(const QString& source, const QString& target, const QString& cmd);

private:
    class Priv_t;
    Priv_t *priv;

    void processResponse(const QJsonObject& o);
};

#endif // CLANGDMANAGER_H
