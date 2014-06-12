#include "documentmanager.h"
#include "codeeditor.h"

#include <QVariant>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>

static inline const QIcon defaultIcon() {
    return QIcon::fromTheme("text-x-c++src");
}

static QIcon unionIcons (const QIcon& icon1, const QIcon& icon2) {
    QSize size = icon1.availableSizes()[0];
    QImage img = QImage(size, QImage::Format_ARGB32);
    img.fill(QColor(0, 0, 0, 0));
    QPainter *painter = new QPainter();

    painter->begin(&img);


    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::HighQualityAntialiasing);

    painter->drawPixmap(0, 0, icon1.pixmap(size));
    painter->drawPixmap(0, 0, icon2.pixmap(size));

    painter->end();

    delete painter;

    return QIcon(QPixmap::fromImage(img));
}

static inline const QIcon modifiedIcon() {
    static QIcon mod;
    if (mod.isNull())
        mod = unionIcons(defaultIcon(), QIcon::fromTheme("task-attention"));
    return mod;
}

DocumentManager::DocumentManager(QWidget *parent) :
    QTabWidget(parent)
{
    setObjectName("tabbar");
    setTabsClosable(true);
    setDocumentMode(true);
    setElideMode(Qt::ElideLeft);
    setIconSize(QSize(32,32));
}

CodeEditor *DocumentManager::codeOf(int n) const
{
    return qobject_cast<CodeEditor*>(widget(n));
}

CodeEditor *DocumentManager::currentCode() const
{
    return qobject_cast<CodeEditor*>(currentWidget());
}

CodeEditor *DocumentManager::newEditor()
{
    CodeEditor *ed = new CodeEditor(this);
    setCurrentIndex(addTab(ed, QString("<New code %1>").arg(count())));
    setTabIcon(currentIndex(), defaultIcon());
    connect(ed, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    return ed;
}

void DocumentManager::openEditor(const QString &fileName)
{
    QFile f(fileName);
    if (f.fileName().isEmpty())
        return;
    if (!f.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("Error open file"), tr("ERROR: %1").arg(f.errorString()));
        return;
    }
    CodeEditor *ed = newEditor();
    if (!ed->read(&f)) {
        ed->deleteLater();
        QMessageBox::critical(this, tr("Error load file"), tr("ERROR: %1").arg(f.errorString()));
        return;
    }
    ed->setFileName(fileName);
    ed->setModified(false);
    int idx = addTab(ed, QFileInfo(f).fileName());
    setTabIcon(idx, defaultIcon());
    setCurrentIndex(idx);
}

bool DocumentManager::saveCode(int idx)
{
    return saveCode(codeOf(idx));
}

bool DocumentManager::saveCode(CodeEditor *ed)
{
    if (!ed) {
        qFatal("null editor");
        return false;
    }
    QString fileName = ed->fileName();
    if (fileName.isEmpty())
        fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return false;
    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        QMessageBox::about(this, tr("File save error"), f.errorString());
        return false;
    }
    if (!ed->write(&f)) {
        QMessageBox::about(this, tr("File save error"), f.errorString());
        return false;
    }
    ed->setFileName(fileName);
    ed->setModified(false);
    handleChangeOf(ed);
    return true;
}

void DocumentManager::closeEditor(int idx)
{
    CodeEditor *ed = codeOf(idx);
    if (ed)
        ed->deleteLater();
    removeTab(idx);
}

void DocumentManager::onTextChanged()
{
    handleChangeOf(qobject_cast<CodeEditor*>(sender()));
}

void DocumentManager::handleChangeOf(CodeEditor *ed)
{
    if (ed) {
        int idx = indexOf(ed);
        if (idx != -1) {
            if (!ed->fileName().isEmpty())
                setTabText(idx, QFileInfo(ed->fileName()).fileName());
            setTabIcon(idx, ed->isModified()? modifiedIcon() : defaultIcon());
        } else
            qWarning("widget index -1");
    } else
        qFatal("Sender is null");
}
