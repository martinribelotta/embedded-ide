/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "appconfig.h"
#include "buildmanager.h"
#include "childprocess.h"
#include "icodemodelprovider.h"
#include "processmanager.h"
#include "projectmanager.h"
#include "regexhtmltranslator.h"
#include "textmessagebrocker.h"

#include <QBuffer>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListView>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeView>

#include <QtDebug>


constexpr size_t operator "" _KB(unsigned long long size) { return static_cast<size_t>(size * 1024); }
constexpr size_t operator "" _MB(unsigned long long size) { return static_cast<size_t>(size * 1024 * 1024); }
constexpr size_t operator "" _GB(unsigned long long size) { return static_cast<size_t>(size * 1024 * 1024 * 1024); }

constexpr auto TARGETVIEW_ICON_SIZE = QSize(16, 16);

const QString SPACE_SEPARATORS = R"(\s)";
const QString DISCOVER_PROC = "makeDiscover";
const QString EXPORT_PROC = "exporter";

using targetMap_t = QHash<QString, QStringList>;

class ProjectManager::Priv_t {
public:
    QStringList targets;
    targetMap_t allTargets;
    targetMap_t allRefs;
    QRegularExpression targetFilter{ R"(^(?!Makefile)[a-zA-Z0-9_\\-]+$)", QRegularExpression::MultilineOption };
    QListView *targetView{ nullptr };
    ProcessManager *pman{ nullptr };
    QFileInfo makeFile;
    ICodeModelProvider *codeModelProvider{ nullptr };
    QTimer clearMessageTimer;

    void doCloseProject() {
        allTargets.clear();
        targets.clear();

        if (targetView->model())
            targetView->model()->deleteLater();
        targetView->setModel(new QStandardItemModel(targetView));

        if (pman->isRunning(DISCOVER_PROC))
            pman->terminate(DISCOVER_PROC, true);
        makeFile = QFileInfo();

        for(auto *p: pman->findChildren<QProcess*>())
            ChildProcess::safeStop(p);
    }
};

static QPair<targetMap_t, targetMap_t> findAllTargets(QIODevice *in)
{
    QPair<targetMap_t, targetMap_t> map;
    QRegularExpression re(R"(^([^\#\s][^\%\=]*?):[^\=]\s*([^#\r\n]*?)\s*$)");
    while (!in->atEnd()) {
        auto line = in->readLine();
        if (line.startsWith("# Not a target:")) {
            in->readLine();
            in->readLine();
            line = in->readLine();
        }
        auto me = re.match(line);
        if (me.hasMatch()) {
            auto tgt = me.captured(1);
            auto depsText = me.captured(2);
            auto deps = depsText.split(' ');
            map.first[tgt].append(deps);
            for(const auto& a: deps)
                map.second[a].append(tgt);
        }
    }
    return map;
}

ProjectManager::ProjectManager(QListView *view, ProcessManager *pman, QObject *parent) :
    QObject(parent),
    priv(std::make_unique<Priv_t>())
{
    priv->targetView = view;
    priv->targetView->setModel(new QStandardItemModel(priv->targetView));
    priv->pman = pman;

    connect(&AppConfig::instance(), &AppConfig::configChanged, [view]() {
        for(auto *button: view->findChildren<QPushButton*>())
            button->setIcon(QIcon(AppConfig::resourceImage({ "actions", "run-build" })));
    });

    auto label = new QLabel(view);
    auto g = new QGridLayout(view);
    g->addWidget(label, 1, 1);
    g->setRowStretch(0, 1);
    g->setColumnStretch(0, 1);
    TextMessageBrocker::instance().subscribe(TextMessages::ACTION_LABEL, [label](const QString& s) {
        label->setVisible(!s.isEmpty());
        label->setText(s);
    });
    connect(&priv->clearMessageTimer, &QTimer::timeout, [this]() { clearMessage(); });
    priv->pman->setTerminationHandler(DISCOVER_PROC, [this](QProcess *make, int code, QProcess::ExitStatus status) {
        Q_UNUSED(code)
        if (status == QProcess::NormalExit) {
            auto res = findAllTargets(make);
            priv->allTargets = res.first;
            priv->allRefs = res.second;
            const auto targetKeys = priv->allTargets.keys();
            priv->targets = targetKeys.filter(priv->targetFilter);
            priv->targets.sort();
            auto targetModel = qobject_cast<QStandardItemModel*>(priv->targetView->model());
            if (targetModel) {
                for(auto& t: priv->targets) {
                    auto item = new QStandardItem;
                    auto button = new QPushButton;
                    auto name = QString(t).replace('_', ' ');
                    targetModel->appendRow(item);
                    button->setIcon(QIcon(AppConfig::resourceImage({ "actions", "run-build" })));
                    button->setIconSize(TARGETVIEW_ICON_SIZE);
                    button->setText(name);
                    button->setStyleSheet("text-align: left; padding: 4px;");
                    priv->targetView->setIndexWidget(item->index(), button);
                    item->setSizeHint(button->sizeHint());
                    connect(button, &QPushButton::clicked, [t, this](){ emit targetTriggered(t); });
                }
            }
        }
        showMessageTimed(tr("Finish target discover"));
    });
}

ProjectManager::~ProjectManager()
{
}

QString ProjectManager::projectName() const
{
    return priv->makeFile.absoluteDir().dirName();
}

QString ProjectManager::projectPath() const
{
    return priv->makeFile.canonicalPath();
}

QString ProjectManager::projectFile() const
{
    return priv->makeFile.canonicalFilePath();
}

bool ProjectManager::isProjectOpen() const
{
    return priv? priv->makeFile.exists() : false;
}

ICodeModelProvider *ProjectManager::codeModel() const
{
    return priv->codeModelProvider;
}

void ProjectManager::setCodeModelProvider(ICodeModelProvider *modelProvider)
{
    priv->codeModelProvider = modelProvider;
}

QStringList ProjectManager::dependenciesForTarget(const QString &target)
{
    return priv->allTargets.value(target);
}

QStringList ProjectManager::targetsOfDependency(const QString &dep)
{
    return priv->allRefs.value(dep);
}

void ProjectManager::createProject(const QString& projectFilePath, const QString& templateFile)
{
    AppConfig::ensureExist(projectFilePath);
    auto onStartCb = [templateFile](QProcess *p)
    {
        p->write(templateFile.toLocal8Bit());
        p->closeWriteChannel();
    };
    auto onFinishCb = [this, projectFilePath](QProcess *p, int exitCode) {
        Q_UNUSED(p)
        TextMessageBrocker::instance().publish(TextMessages::STDOUT_LOG, tr("Diff terminate. Exit code %1").arg(exitCode));
        openProject(QDir(projectFilePath).filePath("Makefile"));
    };
    auto onErrCb = [](QProcess *p, QProcess::ProcessError err) {
        Q_UNUSED(err)
        TextMessageBrocker::instance().publish(TextMessages::STDERR_LOG, tr("Diff error: %1").arg(p->errorString()));
    };
    auto& p = ChildProcess::create(this)
            .makeDeleteLater()
            .changeCWD(projectFilePath)
            .onStarted(onStartCb)
            .onFinished(onFinishCb)
            .onError(onErrCb);
    p.start("patch", { "-p1" });
    deleteOnCloseProject(&p);
}

void ProjectManager::createProjectFromTGZ(const QString &projectFilePath, const QString &tgzFile)
{
    AppConfig::ensureExist(projectFilePath);
    auto onFinishCb = [this, projectFilePath](QProcess *p, int exitCode) {
        Q_UNUSED(p)
        TextMessageBrocker::instance().publish(TextMessages::STDOUT_LOG, tr("tar terminate. Exit code %1").arg(exitCode));
        openProject(QDir(projectFilePath).filePath("Makefile"));
    };
    auto onErrCb = [](QProcess *p, QProcess::ProcessError err) {
        Q_UNUSED(err)
        TextMessageBrocker::instance().publish(TextMessages::STDERR_LOG, tr("tar error: %1").arg(p->errorString()));
    };
    auto& p = ChildProcess::create(this)
        .makeDeleteLater()
        .changeCWD(projectFilePath)
        .onFinished(onFinishCb)
        .onError(onErrCb);
    p.start("tar", { "-x", "-v", "-z", "-f", tgzFile, "-C", projectFilePath });
    deleteOnCloseProject(&p);
}

void ProjectManager::exportCurrentProjectTo(const QString &fileName)
{
    if (QFileInfo(fileName).suffix().compare("template", Qt::CaseInsensitive) == 0) {
        exportToDiff(fileName);
    } else {
        exportToTarGz(fileName);
    }
}

void ProjectManager::exportToDiff(const QString &patchFile)
{
    auto tmpDir = QDir(QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(QString("%1-empty").arg(projectName())));
    if (!tmpDir.exists())
        QDir::root().mkpath(tmpDir.absolutePath());
    priv->pman->setTerminationHandler(EXPORT_PROC, [this, tmpDir, patchFile](QProcess *p, int code, QProcess::ExitStatus status) {
        Q_UNUSED(code)
        QDir::root().rmpath(tmpDir.absolutePath());
        if (status == QProcess::NormalExit) {
            QFile f(patchFile);
            if (f.open(QFile::WriteOnly)) {
                auto BLOCK_SIZE = 1024_KB;
                p->setCurrentReadChannel(QProcess::StandardOutput);
                while (p->bytesAvailable() > 0)
                    f.write(p->read(BLOCK_SIZE));
                f.close();
            }
            if (f.error() == QFile::NoError)
                emit exportFinish(tr("Export sucessfull"));
            else
                emit exportFinish(f.errorString());
        } else
            emit exportFinish(p->errorString());
        p->deleteLater();
    });
    priv->pman->setStderrInterceptor(EXPORT_PROC, [](QProcess* p, const QString& text) {
        QString s{ text };
        TextMessageBrocker::instance()
            .publish(TextMessages::STDERR_LOG,
                     RegexHTMLTranslator::CONSOLE_TRANSLATOR(p, s));
    });
    priv->pman->start(EXPORT_PROC, "diff", { "-N", "-u", "-r", tmpDir.absolutePath(), "." }, {}, projectPath());
    TextMessageBrocker::instance()
        .publish(TextMessages::STDOUT_LOG,
                 tr(R"(<font color="blue">Exporting to %1</font><br>)").arg(patchFile));
}

void ProjectManager::exportToTarGz(const QString &tgzFile)
{
    priv->pman->start(BuildManager::PROCESS_NAME, "tar", { "-c", "-z", "-v", "-f", tgzFile, "." }, {}, projectPath());
    TextMessageBrocker::instance()
        .publish(TextMessages::STDOUT_LOG,
                 tr(R"(<font color="blue">Exporting to %1</font><br>)").arg(tgzFile));
}

void ProjectManager::openProject(const QString &makefile)
{
    auto doOpenProject = [makefile, this]() {
        priv->pman->start(DISCOVER_PROC,
                          "make",
                          { "-B", "-p", "-r", "-n", "-f", makefile },
                          { { "LC_ALL", "C" }, /*{ "LANG", "C" }*/ },
                          QFileInfo(makefile).absolutePath());
        priv->makeFile = QFileInfo(makefile);
        emit projectOpened(makefile);
        showMessageTimed(tr("Discovering targets..."));
        constexpr auto DO_OPEN_DELAY_MS = 100;
        QTimer::singleShot(DO_OPEN_DELAY_MS, [this]() {
            priv->codeModelProvider->startIndexingProject(projectPath(), [this] {
                emit indexFinished();
            });
        });
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
    priv->doCloseProject();
    emit projectClosed();
}

void ProjectManager::reloadProject()
{
    auto project = projectFile();
    priv->doCloseProject();
    openProject(project);
}

void ProjectManager::showMessage(const QString &msg)
{
    priv->clearMessageTimer.stop();
    TextMessageBrocker::instance().publish(TextMessages::ACTION_LABEL, msg);
}

void ProjectManager::showMessageTimed(const QString &msg, int millis)
{
    showMessage(msg);
    clearMessageTimed(millis);
}

void ProjectManager::clearMessage()
{
    TextMessageBrocker::instance().publish(TextMessages::ACTION_LABEL, {});
}

void ProjectManager::clearMessageTimed(int millis)
{
    priv->clearMessageTimer.setInterval(millis);
    QMetaObject::invokeMethod(&priv->clearMessageTimer, "start");
}
