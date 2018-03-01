#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "projectmanager.h"
#include "filesystemmanager.h"

#include <QFileDialog>
#include <QStringListModel>
#include <QScrollBar>

#define CURRENT_VERSION "0.7-pre"

class MainWindow::Priv_t {
public:
    ProjectManager *projectManager;
    FilesystemManager *fileManager;
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
    ui->stackedWidget->setCurrentWidget(ui->welcomePage);
    ui->documentContainer->setComboBox(ui->documentSelector);
    ui->labelVersion->setText(ui->labelVersion->text().replace("{{version}}", CURRENT_VERSION));

    priv->projectManager = new ProjectManager(this);
    priv->projectManager->setTargetView(ui->actionViewer);
    priv->fileManager = new FilesystemManager(ui->fileViewer, this);
    connect(priv->fileManager, &FilesystemManager::requestFileOpen, ui->documentContainer, &DocumentManager::openDocument);

    if (1) {
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
        auto rMargin = ui->logView->verticalScrollBar()->sizeHint().width();
        gl->setContentsMargins(0, 0, rMargin, 0);
    }

    connect(ui->buttonCloseProject, &QToolButton::clicked, priv->projectManager, &ProjectManager::closeProject);
    connect(ui->buttonOpenProject, &QToolButton::clicked, [this]() {
        auto lastDir = QDir::homePath();
        auto path = QFileDialog::getOpenFileName(this, tr("Open Project"), lastDir, tr("Makefile (Makefile);;All files (*)"));
        if (!path.isEmpty()) {
            openProject(path);
        }
    });

    connect(priv->projectManager, &ProjectManager::projectOpened, [this](const QString& makefile) {
        setEnableAllButtonGroup(ui->projectButtons, true);
        ui->stackedWidget->setCurrentWidget(ui->mainPage);
        priv->fileManager->openPath(QFileInfo(makefile).absolutePath());
    });
    connect(priv->projectManager, &ProjectManager::projectClosed, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->welcomePage);
        setEnableAllButtonGroup(ui->projectButtons, false);
        ui->documentContainer->closeAll();
        priv->fileManager->closePath();
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

    connect(ui->buttonConfiguration, &QToolButton::clicked, [this]() { ui->stackedWidget->setCurrentWidget(ui->configPage); });
    connect(ui->buttonConfigAccept, &QToolButton::clicked, [this]() { ui->stackedWidget->setCurrentWidget(ui->welcomePage); });
    connect(ui->buttonConfigReject, &QToolButton::clicked, [this]() { ui->stackedWidget->setCurrentWidget(ui->welcomePage); });

#if 0
    ui->recentProjectsView->setModel(new QStringListModel({
                                                              "Proyecto1",
                                                              "Proyecto2",
                                                              "Proyecto3",
                                                              "Proyecto4",
                                                              "Proyecto5",
                                                              "Proyecto Nro 6",
                                                          }));
#endif
}

MainWindow::~MainWindow()
{
    delete priv;
    delete ui;
}

void MainWindow::openProject(const QString &path)
{
    priv->projectManager->openProject(path);
}
