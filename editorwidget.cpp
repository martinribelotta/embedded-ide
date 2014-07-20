#include "editorwidget.h"
#include "ui_editorwidget.h"
#include "highlighter.h"

#include <QCompleter>
#include <QStringListModel>

#include <QtDebug>

#include <QFileInfo>

EditorWidget::EditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditorWidget)
{
    ui->setupUi(this);
    QAction *saveAction = new QAction(this);
    saveAction->setShortcut(QKeySequence("ctrl+s"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    ui->editor->addAction(saveAction);
    connect(ui->editor, SIGNAL(fileError(QString)), this, SIGNAL(editorError(QString)));
}

EditorWidget::~EditorWidget()
{
    delete ui;
}

static const QStringList C_STYPE = QString("c cpp cxx cc h hh hpp").split(' ');
static const QStringList C_WORDS = QString("if else return goto int void long unsigned return").split(' ');

bool EditorWidget::load(const QString &fileName)
{
    QFileInfo info(fileName);
    if (!ui->editor->load(info.absoluteFilePath()))
        return false;
    setWindowFilePath(info.absoluteFilePath());
    setWindowTitle(info.fileName());
    if (C_STYPE.contains(info.suffix())) {
        new Highlighter(ui->editor->document());
    }
    return true;
}

bool EditorWidget::save()
{
    qDebug() << "****** saving ******";
    return ui->editor->save();
}
