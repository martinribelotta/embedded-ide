#include "projectmanager.h"

#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QListView>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QTreeView>

class ProjectManager::Priv_t {
public:
    QStringList targets;
    QHash<QString, QString> allTargets;
    QRegularExpression targetFilter{ R"(^(?!Makefile)[a-zA-Z0-9_\\-]+$)", QRegularExpression::MultilineOption };
    QListView *targetView = nullptr;
};

ProjectManager::ProjectManager(QObject *parent) :
    QObject(parent),
    priv(new Priv_t)
{
    setProperty("project", QString());
}

ProjectManager::~ProjectManager()
{
    delete priv;
}

void ProjectManager::setTargetView(QListView *view)
{
    priv->targetView = view;
    priv->targetView->setModel(new QStandardItemModel(priv->targetView));
}

static QHash<QString, QString> findAllTargets(const QString& text)
{
    QHash<QString, QString> map;
    QRegularExpression re(R"(^([a-zA-Z0-9 \t\\\/_\.\:\-]*?):(?!=)\s*([^#\r\n]*?)\s*$)",
                          QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext()) {
        QRegularExpressionMatch me = it.next();
        map.insert(me.captured(1), me.captured(2));
    }
    return map;
}

void ProjectManager::openProject(const QString &makefile)
{
    auto doOpenProject = [makefile, this]() {
        auto make = new QProcess(this);
        make->setObjectName("makeProcess");
        make->setWorkingDirectory(QFileInfo(makefile).absolutePath());
        auto env = make->processEnvironment();
        // Important: Set LANG to C for non-tr messages
        env.insert("LC_ALL", "C");
        make->setProcessEnvironment(env);
        make->setProgram("make");
        make->setArguments({ "-B", "-p", "-r", "-n", "-f", makefile });
        connect(make, &QProcess::errorOccurred, [make]() { make->deleteLater(); });
        connect(make, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                [make, this](int code, QProcess::ExitStatus status) {
            Q_UNUSED(code);
            if (status == QProcess::NormalExit) {
                auto out = QString(make->readAllStandardOutput());
                priv->allTargets = findAllTargets(out);
                priv->targets = priv->allTargets.keys().filter(priv->targetFilter);
                priv->targets.sort();
                auto targetModel = qobject_cast<QStandardItemModel*>(priv->targetView->model());
                if (targetModel) {
                    for(auto& t: priv->targets) {
                        auto item = new QStandardItem;
                        auto *button = new QPushButton;
                        targetModel->appendRow(item);
                        button->setIcon(QIcon(":/images/actions/run-build.svg"));
                        button->setIconSize(QSize(32, 32));
                        button->setText(t);
                        button->setStyleSheet("text-align: left; padding: 4px;");
                        priv->targetView->setIndexWidget(item->index(), button);
                        item->setSizeHint(button->sizeHint());
                        connect(button, &QPushButton::clicked, [t, this](){ emit targetTriggered(t); });
                    }
                }
            }
            make->deleteLater();
        });
        make->start();
        setProperty("project", makefile);
        emit projectOpened(makefile);
    };

    // FIXME: Unnecesary if force to close project after open other
    if (isProjectOpen()) {
        auto con = connect(this, &ProjectManager::projectClosed, doOpenProject);
        connect(this, &ProjectManager::projectClosed, [con]() { disconnect(con); });
        closeProject();
    } else
        doOpenProject();
}

void ProjectManager::closeProject()
{
    priv->allTargets.clear();
    priv->targets.clear();

    if (priv->targetView->model())
        priv->targetView->model()->deleteLater();
    priv->targetView->setModel(new QStandardItemModel(priv->targetView));

    auto make = findChild<QProcess*>("makeProcess");
    if (make) {
        make->terminate();
        if (!make->waitForFinished(1000))
            make->kill();
        make->deleteLater();
    }
    setProperty("project", QString());
    emit projectClosed();
}
