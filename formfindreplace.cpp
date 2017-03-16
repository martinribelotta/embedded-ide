#include "formfindreplace.h"
#include "ui_formfindreplace.h"

FormFindReplace::FormFindReplace(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormFindReplace)
{
    ui->setupUi(this);
    setAutoFillBackground(true);
}

FormFindReplace::~FormFindReplace()
{
    delete ui;
}
