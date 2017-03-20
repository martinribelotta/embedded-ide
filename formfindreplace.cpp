#include "codeeditor.h"
#include "formfindreplace.h"
#include "ui_formfindreplace.h"

#include <QKeyEvent>

FormFindReplace::FormFindReplace(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormFindReplace)
{
    ui->setupUi(this);
    setAutoFillBackground(true);
    ui->textToFind->addMenuAction(QHash<QString, QString>{
        { tr("Regular Expression"), "regex" },
        { tr("Case Sensitive"), "case" },
        { tr("Wole Words"), "wword" },
        { tr("Selection Only"), "selonly" },
    });
    ui->textToReplace->addMenuAction(QHash<QString, QString>{
        { tr("Replace All"), "replaceAll" }
    });
}

FormFindReplace::~FormFindReplace()
{
    delete ui;
}

void FormFindReplace::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    ui->textToFind->setFocus();
}
