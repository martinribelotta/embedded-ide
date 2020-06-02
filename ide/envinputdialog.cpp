#include "envinputdialog.h"
#include "ui_envinputdialog.h"

#include "appconfig.h"

#include <QFileDialog>
#include <QMenu>
#include <QProcessEnvironment>

template<typename F>
static QAction *mkAction(const QString& actionName, F func)
{
    auto a = new QAction{QIcon{AppConfig::resourceImage({ "actions", actionName })}, actionName};
    QObject::connect(a, &QAction::triggered, func);
    return a;
}

static QAction *mkAction(const QString& actionName, QMenu *m)
{
    auto a = new QAction{QIcon{AppConfig::resourceImage({ "actions", actionName })}, actionName};
    a->setMenu(m);
    return a;
}

template<typename F>
static QMenu *menuFromEnv(QWidget *parent, F func)
{
    auto m = new QMenu(parent);
    auto env = QProcessEnvironment::systemEnvironment();
    for (const auto& k: env.keys()) {
        const auto v = env.value(k);
        auto a = m->addAction(k);
        a->setData(k);
    }
    QObject::connect(m, &QMenu::triggered, func);
    return m;
}

EnvInputDialog::EnvInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnvInputDialog)
{
    ui->setupUi(this);
    ui->valueEditor->addAction(mkAction("document-open", [this]() { openPathSelect(); }), QLineEdit::TrailingPosition);
    ui->valueEditor->addAction(mkAction("code-context", menuFromEnv(this, [this](QAction *a) { menuAction(a); })), QLineEdit::TrailingPosition);
}

EnvInputDialog::~EnvInputDialog()
{
    delete ui;
}

QString EnvInputDialog::envName() const
{
    return ui->nameEditor->text();
}

QString EnvInputDialog::envValue() const
{
    return ui->valueEditor->text();
}

void EnvInputDialog::openPathSelect()
{
    auto currPath = ui->valueEditor->text();
    if (currPath.isEmpty())
        currPath = QDir::homePath();
    auto newPath = QFileDialog::getExistingDirectory(window(), tr("Select directory"), currPath);
    if (!newPath.isEmpty()) {
        ui->valueEditor->insert(newPath);
    }
}

void EnvInputDialog::openVarSelect()
{
    // TODO
}

void EnvInputDialog::menuAction(QAction *a)
{
    ui->valueEditor->insert(QString("${%1}").arg(a->text()));
}
