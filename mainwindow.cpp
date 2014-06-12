#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "codeeditor.h"
#include "configuredialog.h"
#include "buildmanager.h"

#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    runner = new BuildManager(this);
    ui->setupUi(this);
    toggle_lockWidgets(true);
    ui->textBrowser->setFont(CodeEditor::defaultFont());

    runner->setStderr(ui->textBrowser);
    runner->setStdout(ui->textBrowser);

    ui->actionNew->trigger();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QMenu *MainWindow::createPopupMenu()
{
    QMenu *m = new QMenu(this);
    bool locked = property("locked").toBool();

    QList<QToolBar*> toolbarList = findChildren<QToolBar*>();
    foreach(QToolBar *tb, toolbarList) {
        QAction *a = m->addAction(tr("Show/Hide %1").arg(tb->windowTitle()));
        a->setEnabled(!locked);
        a->setCheckable(true);
        a->setChecked(tb->isVisible());
        connect(a, SIGNAL(toggled(bool)), tb, SLOT(setVisible(bool)));
    }

    m->addSeparator();

    QList<QDockWidget*> docks = findChildren<QDockWidget*>();
    foreach(QDockWidget *dk, docks) {
        QAction *a = m->addAction(tr("Show/Hide %1").arg(dk->windowTitle()));
        a->setEnabled(!locked);
        a->setCheckable(true);
        a->setChecked(dk->isVisible());
        connect(a, SIGNAL(toggled(bool)), dk, SLOT(setVisible(bool)));
    }
    m->addSeparator();

    QAction *lock = m->addAction(tr("Lock/Unlock widgets"));
    lock->setCheckable(true);
    lock->setChecked(property("locked").toBool());
    connect(lock, SIGNAL(toggled(bool)), this, SLOT(toggle_lockWidgets(bool)));
    return m;
}

void MainWindow::on_centralWidget_tabCloseRequested(int n)
{
    CodeEditor *ed = ui->centralWidget->codeOf(n);
    if (ed) {
        if (ed->isModified())
            switch(QMessageBox::question(this, tr("File modified"), tr("File as been modified. Save it?"),
                                      QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) {
            case QMessageBox::Yes:
                if (!ui->centralWidget->saveCode(n))
                    return;
                break;
            case QMessageBox::No:
                break;
            case QMessageBox::Cancel:
                return;
            default:
                qWarning("Unexpected dialog return");
                return;
            }
        ui->centralWidget->closeEditor(n);
    } else
        qFatal("Tab %d not exists", n);
}

void MainWindow::on_actionNew_triggered()
{
    ui->centralWidget->newEditor();
}

void MainWindow::on_actionOpen_triggered()
{
    ui->centralWidget->openEditor(QFileDialog::getOpenFileName(this));
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionSave_triggered()
{
    ui->centralWidget->saveCode(ui->centralWidget->currentIndex());
}

static QString resolve(QString cmd, const CodeEditor::Context_t& ctx) {
    foreach(QString key, ctx.keys())
        cmd.replace(QString("$(%1)").arg(key), ctx.value(key));
    return cmd;
}

void MainWindow::on_actionBuild_triggered()
{
    CodeEditor *ed = ui->centralWidget->currentCode();
    if (ed) {
        if (ed->fileName().isEmpty() || ed->isModified())
            if (!ui->centralWidget->saveCode(ed))
                return;
        CodeEditor::Context_t ctx = ui->centralWidget->currentCode()->context();
        runner->start(resolve(ProgrammSettings().buildCommand(), ctx));
    }
}

void MainWindow::on_actionConfigure_triggered()
{
    switch(ConfigureDialog(this).exec()) {
    default:
        break;
    }
}

void MainWindow::on_textBrowser_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu;
    menu->addAction(tr("Copy"), ui->textBrowser, SLOT(copy()))->setDisabled(ui->textBrowser->textCursor().selectedText().isEmpty());
    menu->addAction(tr("Select All"), ui->textBrowser, SLOT(selectAll()));
    menu->addSeparator();
    menu->addAction(tr("Clear"), ui->textBrowser, SLOT(clear()));
    menu->exec(qobject_cast<QWidget*>(sender())->mapToGlobal(pos));
}

void MainWindow::toggle_lockWidgets(bool lock)
{
    setProperty("locked", lock);
    foreach(QToolBar *w, findChildren<QToolBar*>())
        w->setMovable(!lock);
    foreach(QDockWidget *w, findChildren<QDockWidget*>())
        w->setFeatures(lock? QDockWidget::DockWidgetFeature(0) :
            (QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetMovable));
}
