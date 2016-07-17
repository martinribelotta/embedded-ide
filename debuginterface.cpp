#include "debuginterface.h"
#include "ui_debuginterface.h"

DebugInterface::DebugInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugInterface)
{
    ui->setupUi(this);
}

DebugInterface::~DebugInterface()
{
    delete ui;
}
