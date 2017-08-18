#include "debugtoolbar.h"
#include "ui_debugtoolbar.h"

#include <QShortcut>

DebugToolBar::DebugToolBar(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DebugToolBar)
{
    setAutoFillBackground(true);
    ui->setupUi(this);
    auto scNext = new QShortcut(QKeySequence("F5"), this);
    auto scNextIn = new QShortcut(QKeySequence("F6"), this);
    auto scRunToEnd = new QShortcut(QKeySequence("F7"), this);
    shortcuts = { scNext, scNextIn, scRunToEnd };
    for(auto a: shortcuts) a->setContext(Qt::ApplicationShortcut);
    connect(ui->toolStepOver, &QToolButton::clicked, this, &DebugToolBar::debugStepOver);
    connect(ui->toolStepInto, &QToolButton::clicked, this, &DebugToolBar::debugStepInto);
    connect(ui->toolRunToEnd, &QToolButton::clicked, this, &DebugToolBar::debugStepOut);
    connect(scNext, &QShortcut::activated, this, &DebugToolBar::debugStepOver);
    connect(scNextIn, &QShortcut::activated, this, &DebugToolBar::debugStepInto);
    connect(scRunToEnd, &QShortcut::activated, this, &DebugToolBar::debugStepOut);
}

DebugToolBar::~DebugToolBar()
{
    delete ui;
}

void DebugToolBar::showEvent(QShowEvent *event)
{
    for(auto a: shortcuts) a->setEnabled(true);
    QFrame::showEvent(event);
}

void DebugToolBar::hideEvent(QHideEvent *event)
{
    for(auto a: shortcuts) a->setEnabled(false);
    QFrame::hideEvent(event);
}
