#ifndef COMBODOCUMENTVIEW_H
#define COMBODOCUMENTVIEW_H

#include <QAction>
#include <QWidget>

class ComboDocumentView : public QWidget
{
    Q_OBJECT

public:
    explicit ComboDocumentView(QWidget *parent = 0);
    ~ComboDocumentView();

    void addWidgetToLeftCorner(QWidget *w);
    void addWidgetToRigthCorner(QWidget *w);

    void addActionToLeftCorner(QAction *a);
    QAction *addActionToLeftCorner(const QIcon& icon, QObject *recv, const char *slot_name) {
        auto a = new QAction(icon, QString(), this);
        addActionToLeftCorner(a);
        connect(a, SIGNAL(triggered()), recv, slot_name);
        return a;
    }

    void addActionToRightCorner(QAction *a);
    QAction *addActionToRightCorner(const QIcon& icon, QObject *recv, const char *slot_name) {
        auto a = new QAction(icon, QString(), this);
        addActionToRightCorner(a);
        connect(a, SIGNAL(triggered()), recv, slot_name);
        return a;
    }

    int addWidget(QWidget *w, const QString& title);
    void removeWidget(QWidget *w);
    void removeWidget(int idx);

    QWidget *currentWidget() const;
    int currentIndex() const;
    int widgetCount() const;
    QWidget *widget(int idx) const;
    int index(QWidget *w) const;
    QString widgetTitle(int idx) const;
    QString widgetTitle(QWidget *w) const;

    void setCurrentIndex(int idx);
    void setCurrentWidget(QWidget *w);

    void setWidgetTitle(int idx, const QString& title);
    void setWidgetTitle(QWidget *w, const QString& title);

signals:
    void widgetAdded(int idx, QWidget *w);
    void widgetRemoved(int idx, QWidget *w);
    void widgetCurrentChanged(int idx, QWidget *w);

private slots:
    void on_stackedWidget_currentChanged(int index);

    void on_stackedWidget_widgetRemoved(int index);

    void on_comboDocuments_currentIndexChanged(int index);

    void on_comboDocuments_activated(int index);

private:
    struct Priv_t;
    Priv_t *priv;

    int comboIndexFromStackIndex(int idx) const;
    int comboIndexFromStackWidget(QWidget *w) const;
    int stackIndexFromComboIndex(int idx) const;
    QWidget *stackWidgetFromComboIndex(int idx) const;
};

#endif // COMBODOCUMENTVIEW_H
