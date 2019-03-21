#include "combodocumentview.h"

#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QtDebug>

#if 0
#define LOG(...) do { qDebug() << __func__ << __LINE__ << ": " << __VA_ARGS__; } while(0)
#else
#define LOG(...) ((void)0)
#endif

struct ComboDocumentView::Priv_t {
    QHBoxLayout *horizontalLayout;
    QStackedWidget *stackedWidget;
    QComboBox *comboDocuments;
};

ComboDocumentView::ComboDocumentView(QWidget *parent) :
    QWidget(parent),
    priv(new ComboDocumentView::Priv_t)
{
    QVBoxLayout *verticalLayout;
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(0);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    priv->horizontalLayout = new QHBoxLayout();
    priv->horizontalLayout->setSpacing(0);
    priv->horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    priv->comboDocuments = new QComboBox(this);
    priv->comboDocuments->setObjectName(QStringLiteral("comboDocuments"));

    priv->horizontalLayout->addWidget(priv->comboDocuments);
    verticalLayout->addLayout(priv->horizontalLayout);

    priv->stackedWidget = new QStackedWidget(this);
    priv->stackedWidget->setObjectName(QStringLiteral("stackedWidget"));

    verticalLayout->addWidget(priv->stackedWidget);

    priv->comboDocuments->setEnabled(false);

    QMetaObject::connectSlotsByName(this);
}

ComboDocumentView::~ComboDocumentView()
{
    delete priv;
}

void ComboDocumentView::addWidgetToLeftCorner(QWidget *w)
{
    priv->horizontalLayout->insertWidget(0, w);
    w->setParent(this);
}

void ComboDocumentView::addWidgetToRigthCorner(QWidget *w)
{
    priv->horizontalLayout->addWidget(w);
    w->setParent(this);
}

void ComboDocumentView::addActionToLeftCorner(QAction *a)
{
    QToolButton *b = new QToolButton(this);
    b->setDefaultAction(a);
    b->setAutoRaise(true);
    addWidgetToLeftCorner(b);
}

void ComboDocumentView::addActionToRightCorner(QAction *a)
{
    QToolButton *b = new QToolButton(this);
    b->setDefaultAction(a);
    b->setAutoRaise(true);
    addWidgetToRigthCorner(b);
}

int ComboDocumentView::addWidget(QWidget *w, const QString &title)
{
    int idx = priv->stackedWidget->addWidget(w);
    priv->comboDocuments->addItem(title, QVariant::fromValue(w));
    priv->comboDocuments->setEnabled(true);
    emit widgetAdded(idx, w);
    LOG(idx);
    return idx;
}

void ComboDocumentView::removeWidget(QWidget *w)
{
    int idx = priv->stackedWidget->indexOf(w);
    LOG(idx);
    priv->stackedWidget->removeWidget(w);
    priv->comboDocuments->setEnabled(widgetCount() > 0);
    emit widgetRemoved(idx, w);
}

void ComboDocumentView::removeWidget(int idx)
{
    QWidget *w = priv->stackedWidget->widget(idx);
    if (w) {
        LOG(idx);
        bool s;
        s = priv->stackedWidget->blockSignals(true);
        priv->stackedWidget->removeWidget(w);
        priv->stackedWidget->blockSignals(s);
        s = priv->comboDocuments->blockSignals(true);
        int comboIdx = comboIndexFromStackWidget(w);
        if (comboIdx != -1)
            priv->comboDocuments->removeItem(comboIdx);
        else
            LOG("comboIdx -1");
        priv->comboDocuments->blockSignals(s);
        priv->comboDocuments->setEnabled(widgetCount() > 0);
        emit widgetRemoved(idx, w);
    }
}

QWidget *ComboDocumentView::currentWidget() const
{
    return priv->stackedWidget->currentWidget();
}

int ComboDocumentView::currentIndex() const
{
    return priv->stackedWidget->currentIndex();
}

int ComboDocumentView::widgetCount() const
{
    return priv->stackedWidget->count();
}

QWidget *ComboDocumentView::widget(int idx) const
{
    return priv->stackedWidget->widget(idx);
}

int ComboDocumentView::index(QWidget *w) const
{
    return priv->stackedWidget->indexOf(w);
}

QString ComboDocumentView::widgetTitle(int idx) const
{
    int comboIndex = comboIndexFromStackIndex(idx);
    return priv->comboDocuments->itemText(comboIndex);
}

QString ComboDocumentView::widgetTitle(QWidget *w) const
{
    int comboIndex = comboIndexFromStackWidget(w);
    return priv->comboDocuments->itemText(comboIndex);
}

void ComboDocumentView::setCurrentIndex(int idx)
{
    priv->stackedWidget->setCurrentIndex(idx);
}

void ComboDocumentView::setCurrentWidget(QWidget *w)
{
    priv->stackedWidget->setCurrentWidget(w);
}

void ComboDocumentView::setWidgetTitle(int idx, const QString &title)
{
    setWidgetTitle(widget(idx), title);
}

void ComboDocumentView::setWidgetTitle(QWidget *w, const QString &title)
{
    int comboIndex = comboIndexFromStackWidget(w);
    if (comboIndex != -1) {
        priv->comboDocuments->setItemText(comboIndex, title);
    } else
        LOG("comboIndex == -1");
}

bool ComboDocumentView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowActivate:
        if (currentWidget())
            currentWidget()->setFocus();
        break;
    }
    return QWidget::event(event);
}

void ComboDocumentView::on_stackedWidget_currentChanged(int index)
{
    int comboIndex = comboIndexFromStackIndex(index);
    LOG(QString("index: %1 comboIndex: %2").arg(index).arg(comboIndex));
    if (comboIndex == -1)
        return;
    priv->comboDocuments->setCurrentIndex(comboIndex);
    emit widgetCurrentChanged(index, priv->stackedWidget->widget(index));
}

void ComboDocumentView::on_stackedWidget_widgetRemoved(int index)
{
    int comboIndex = comboIndexFromStackIndex(index);
    QWidget *w = stackWidgetFromComboIndex(comboIndex);
    LOG(QString("index: %1 comboIndex: %2").arg(index).arg(comboIndex));
    if (comboIndex == -1)
        return;
    priv->comboDocuments->removeItem(comboIndex);
    emit widgetRemoved(index, w);
}

void ComboDocumentView::on_comboDocuments_currentIndexChanged(int index)
{
    int comboIndex = comboIndexFromStackIndex(index);
    LOG(QString("index: %1 comboIndex: %2").arg(index).arg(comboIndex));
}

void ComboDocumentView::on_comboDocuments_activated(int index)
{
    int didx = stackIndexFromComboIndex(index);
    LOG(QString("comboIndex: %1 stackIndex: %2").arg(index).arg(didx));
    if (didx == -1)
        return;
    priv->stackedWidget->setCurrentIndex(didx);
}

int ComboDocumentView::comboIndexFromStackIndex(int idx) const
{
    return comboIndexFromStackWidget(priv->stackedWidget->widget(idx));
}

int ComboDocumentView::comboIndexFromStackWidget(QWidget *w) const
{
    auto vidx = QVariant::fromValue(w);
    return priv->comboDocuments->findData(vidx);
}

int ComboDocumentView::stackIndexFromComboIndex(int idx) const
{
    return priv->stackedWidget->indexOf(stackWidgetFromComboIndex(idx));
}

QWidget *ComboDocumentView::stackWidgetFromComboIndex(int idx) const
{
    return priv->comboDocuments->itemData(idx).value<QWidget*>();
}
