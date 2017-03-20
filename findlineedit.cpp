#include "findlineedit.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>
#include <QStyle>

static const QString INNER_BUTTON_STYLE = "background: transparent; border: none;";

static QAction *actionCheckeable(QAction *a) {
    a->setCheckable(true);
    return a;
}

#define ACTION_CB(a, text, prop) do { \
        connect( \
            actionCheckeable(menu->addAction(text)), &QAction::triggered, \
            [this](bool ck) { setProperty(prop, QVariant(ck)); }); \
        setProperty(prop, false); \
    } while(0)

FindLineEdit::FindLineEdit(QWidget *parent) : QLineEdit(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    QToolButton *clearButton = new QToolButton(this);
    optionsButton = new QToolButton(this);
    optionsButton->setIcon(QIcon(":/images/application-menu.svg"));
    clearButton->setIcon(QIcon(":/images/actions/edit-clear.svg"));
    optionsButton->setFocusPolicy(Qt::NoFocus);
    optionsButton->setPopupMode(QToolButton::InstantPopup);
    optionsButton->setCursor(Qt::ArrowCursor);
    clearButton->setFocusPolicy(Qt::NoFocus);
    clearButton->setCursor(Qt::ArrowCursor);
    optionsButton->setStyleSheet(INNER_BUTTON_STYLE);
    clearButton->setStyleSheet(INNER_BUTTON_STYLE);
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; padding-left: %2px }\n"
                          "QToolButton::menu-indicator { image: none; }")
                  .arg(clearButton->sizeHint().width() + frameWidth + 1)
                  .arg(optionsButton->sizeHint().width() + frameWidth + 1));
    layout->addWidget(optionsButton, 0, Qt::AlignLeft);
    layout->addWidget(clearButton, 0, Qt::AlignRight);
    layout->setMargin(0);
    layout->setSpacing(0);
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

void FindLineEdit::addMenuAction(const QHash<QString, QString> &actionList)
{
    QMenu *menu = new QMenu(this);
    QHash<QString, QString>::const_iterator it;
    for (it = actionList.constBegin(); it != actionList.constEnd(); ++it) {
        QString action = it.key();
        QString prop = it.value();
        connect(actionCheckeable(menu->addAction(action)), &QAction::triggered,
                [this, prop](bool ck) {
            setProperty(prop.toLatin1().constData(), QVariant(ck));
        });
        setProperty(prop.toLatin1().constData(), false);
    }
    optionsButton->setMenu(menu);
}
