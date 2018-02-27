#include "projecticonprovider.h"
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

    QTreeView *fileView = nullptr;
    QListView *targetView = nullptr;
};

ProjectManager::ProjectManager(QObject *parent) :
    QObject(parent),
    priv(new Priv_t)
{
}

ProjectManager::~ProjectManager()
{
    delete priv;
}

void ProjectManager::setFileView(QTreeView *view)
{
    if (priv->fileView)
        priv->fileView->disconnect(this);
    priv->fileView = view;
    connect(priv->fileView, &QTreeView::activated, [this](const QModelIndex& idx) {
        auto model = qobject_cast<QFileSystemModel*>(priv->fileView->model());
        if (model) {
            auto path = model->filePath(idx);
            emit requestFileOpen(path);
        }
    });
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
    closeProject();
    auto make = new QProcess(this);
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
    if (priv->fileView) {
        auto model = qobject_cast<QFileSystemModel*>(priv->fileView->model());
        if (!model) {
            if (priv->fileView->model())
                priv->fileView->model()->deleteLater();
            priv->fileView->setModel(model = new QFileSystemModel(priv->fileView));
        }
        model->setIconProvider(new ProjectIconProvider);
        priv->fileView->setRootIndex(model->setRootPath(make->workingDirectory()));
        for(int i=1; i<model->columnCount(); i++)
            priv->fileView->hideColumn(i);
        priv->fileView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        priv->fileView->header()->hide();
    }
    make->start();
    emit projectOpened(makefile);
}

void ProjectManager::closeProject()
{
    priv->allTargets.clear();
    priv->targets.clear();

    if (priv->fileView->model())
        priv->fileView->model()->deleteLater();
    priv->fileView->setModel(nullptr);

    if (priv->targetView->model())
        priv->targetView->model()->deleteLater();
    priv->targetView->setModel(new QStandardItemModel(priv->targetView));

    emit projectClosed();
}
