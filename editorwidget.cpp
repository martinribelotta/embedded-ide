#include "editorwidget.h"
#include "ui_editorwidget.h"
//#include "highlighter.h"

#include "qsvsh/qsvcolordef.h"
#include "qsvsh/qsvcolordeffactory.h"
#include "qsvsh/qsvlangdef.h"
#include "qsvsh/qsvsyntaxhighlighter.h"

#include <QDir>
#include <QSettings>
#include <QCompleter>
#include <QStringListModel>

#include <QtDebug>

#include <QFileInfo>

EditorWidget::EditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditorWidget),
    defColors(0l),
    langDef(0l),
    syntax(0l)
{
    ui->setupUi(this);
    QAction *saveAction = new QAction(this);
    saveAction->setShortcut(QKeySequence("ctrl+s"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));
    ui->editor->addAction(saveAction);
    QFont f(QSettings().value("editor/font/style", "DejaVu Sans Mono").toString(),
            QSettings().value("editor/font/size", 10).toInt());
    ui->editor->setFont(f);
    connect(ui->editor, SIGNAL(fileError(QString)), this, SIGNAL(editorError(QString)));
}

EditorWidget::~EditorWidget()
{
    delete ui;
}

static const QStringList C_STYPE = QString("c cpp cxx cc h hh hpp").split(' ');
static const QStringList C_WORDS = QString("if else return goto int void long unsigned return").split(' ');

static QString findStyleByName(const QString& defaultName) {
    QDir d(":/qsvsh/qtsourceview/data/colors/");
    foreach(QString name, d.entryList(QStringList("*.xml"))) {
        QDomDocument doc("mydocument");
        QFile file(d.filePath(name));
        if (file.open(QIODevice::ReadOnly) && doc.setContent(&file)) {
            QDomNodeList itemDatas = doc.elementsByTagName("itemDatas");
            if (!itemDatas.isEmpty()) {
                QDomNamedNodeMap attr = itemDatas.at(0).attributes();
                QString name = attr.namedItem("name").toAttr().value();
                if (defaultName == name)
                    return file.fileName();
            }
        }
    }
    return QString();
}

bool EditorWidget::load(const QString &fileName)
{
    QFileInfo info(fileName);
    if (!ui->editor->load(info.absoluteFilePath()))
        return false;
    setWindowFilePath(info.absoluteFilePath());
    setWindowTitle(info.fileName());
    if (defColors)
        delete defColors;
    if (langDef)
        delete langDef;
    if (syntax)
        syntax->deleteLater();
    langDef = 0l;
    syntax = 0l;

    defColors = new QsvColorDefFactory( findStyleByName(QSettings().value("editor/colorstyle", "Kate").toString()) );
    if (C_STYPE.contains(info.suffix())) {
        langDef   = new QsvLangDef( ":/qsvsh/qtsourceview/data/langs/cpp.lang" );
    } else if (info.baseName().toLower() == "makefile" || info.suffix().toLower() == ".mk") {
        langDef   = new QsvLangDef( ":/qsvsh/qtsourceview/data/langs/makefile.lang" );
    }
    if (defColors && langDef) {
        syntax = new QsvSyntaxHighlighter( ui->editor->document() , defColors, langDef );
        QPalette p = ui->editor->palette();
        p.setColor(QPalette::Base, defColors->getColorDef("dsWidgetBackground").getBackground());
        ui->editor->setPalette(p);
    }
    return true;
}

bool EditorWidget::save()
{
    qDebug() << "****** saving ******";
    return ui->editor->save();
}
