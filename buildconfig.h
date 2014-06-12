#ifndef BUILDCONFIG_H
#define BUILDCONFIG_H

#include <QWidget>

class BuildConfig : public QWidget
{
    Q_OBJECT
public:
    explicit BuildConfig(QWidget *parent = 0);
    virtual ~BuildConfig();

signals:

public slots:
    void save();
    void load();

private:
    struct Priv_t;
    Priv_t *priv;
};

#endif // BUILDCONFIG_H
