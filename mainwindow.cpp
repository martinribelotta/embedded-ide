#include "mainwindow.h"

#include "ui_mainwindow.h"

#include "appmenu.h"
#include "projectmanager.h"

#include <QFileDialog>

class MainWindow::Priv_t {
public:
    ProjectManager *projectManager;
};

static void setEnableAllButtonGroup(QButtonGroup *b, bool en) {
    for(auto& btn: b->buttons())
        btn->setEnabled(en);
}

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    priv(new Priv_t)
{
    ui->setupUi(this);
    ui->documentContainer->setComboBox(ui->documentSelector);

    priv->projectManager = new ProjectManager(this);
    priv->projectManager->setFileView(ui->fileViewer);
    priv->projectManager->setTargetView(ui->actionViewer);

    auto gl = new QGridLayout(ui->logView);
    auto bclr = new QToolButton(ui->logView);
    bclr->setIcon(QIcon(":/images/actions/edit-clear.svg"));
    bclr->setAutoRaise(true);
    bclr->setIconSize(QSize(32, 32));

    auto bstop = new QToolButton(ui->logView);
    bstop->setEnabled(false);
    bstop->setIcon(QIcon(":/images/actions/window-close.svg"));
    bstop->setAutoRaise(true);
    bstop->setIconSize(QSize(32, 32));

    gl->addWidget(bclr,  0, 1);
    gl->addWidget(bstop, 1, 1);
    gl->setColumnStretch(0, 1);
    gl->setRowStretch(2, 1);
    gl->setMargin(0);

    auto appMenu = new AppMenu;
    ui->buttonMainMenu->setMenu(appMenu->menu(this));
    connect(appMenu, &AppMenu::openAction, this, &MainWindow::openProject);
    connect(appMenu, &AppMenu::closeAction, priv->projectManager, &ProjectManager::closeProject);

    connect(priv->projectManager, &ProjectManager::projectOpened, [this]() {
        setEnableAllButtonGroup(ui->projectButtons, true);
        //setEnableAllButtonGroup(ui->editionButtons, true);
        //ui->documentSelector->setEnabled(true);
    });
    connect(priv->projectManager, &ProjectManager::projectClosed, [this]() {
        setEnableAllButtonGroup(ui->projectButtons, false);
        //setEnableAllButtonGroup(ui->editionButtons, false);
        //ui->documentSelector->setEnabled(false);
    });

    auto enableEdition = [this]() {
        bool en = ui->documentContainer->documentCount() > 0;
        setEnableAllButtonGroup(ui->editionButtons, en);
        ui->documentSelector->setEnabled(en);
    };

    connect(ui->documentContainer, &DocumentManager::documentFocushed, enableEdition);
    connect(ui->documentContainer, &DocumentManager::documentClosed, enableEdition);

    connect(priv->projectManager, &ProjectManager::requestFileOpen, ui->documentContainer, &DocumentManager::openDocument);
    connect(ui->buttonDocumentClose, &QToolButton::clicked, ui->documentContainer, &DocumentManager::closeCurrent);
    connect(ui->buttonDocumentCloseAll, &QToolButton::clicked, ui->documentContainer, &DocumentManager::closeAll);
}

MainWindow::~MainWindow()
{
    delete priv;
    delete ui;
}

void MainWindow::openProject()
{
    auto lastDir = QDir::homePath();
    auto path = QFileDialog::getOpenFileName(this, tr("Open Project"), lastDir, tr("Makefile (Makefile);;All files (*)"));
    if (!path.isEmpty()) {
        priv->projectManager->openProject(path);
    }
}
