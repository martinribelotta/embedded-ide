#include "newprojectfromremotedialog.h"
#include "ui_newprojectfromremotedialog.h"

#include "appconfig.h"
#include "childprocess.h"
#include "consoleinterceptor.h"

#include <QFileDialog>
#include <QMenu>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QDir>

#include <mustache.h>

enum UrlType {
    URL_Git,
    URL_Svn,
    URL_Archive,
};

struct NewProjectFromRemoteDialog::Priv_t
{
    ProcessStatus status = NotStarted;
    UrlType currentType = URL_Git;
    QNetworkAccessManager *net;
};

template<typename E, typename Func>
static QPair<QMenu*, QMap<E, QAction*> > makeMenuAction(QWidget *parent,
                               const QList< QPair<QString, E> > &actions,
                               Func f)
{
    QMap<E, QAction *> revMap;
    auto m = new QMenu(parent);
    auto g = new QActionGroup(m);
    for (const QPair<QString, E>& e: actions) {
        auto ac = m->addAction(QString(e.first), [f, t=e.second](){ f(t); });
        ac->setCheckable(true);
        g->addAction(ac);
        revMap.insert(e.second, ac);
    }
    g->setExclusive(true);
    return { m, revMap };
}

static const QHash<UrlType, QString> urlTypeIcons = {
    { URL_Git, "git" },
    { URL_Svn, "svn" },
    { URL_Archive, "archive" },
};

static const QHash<UrlType, QString> commandForType = {
    { URL_Git, "git clone --verbose --progress {{url}} {{path}}" },
    { URL_Svn, "svn checkout {{url}} {{path}}" },
};

static const QHash<QString, QString> commandForFileType = {
    { "zip", "busybox unzip -d {{path}} -" },
    { "tar.gz", "tar x -zvf - -C {{path}}/../" },
    { "tgz", "tar x -zvf - -C {{path}}/../" },
    { "tar.bz", "tar x -jvf - -C {{path}}/../" },
    { "tar.xz", "tar x -Hvf - -C {{path}}/../" },
};

static UrlType detectUrlType(const QString& urlPath, UrlType defaultValue)
{
    QUrl url{urlPath};
    if (url.scheme().toLower() == "svn") {
        return URL_Svn;
    }
    if (url.scheme().toLower() == "git") {
        return URL_Git;
    }
    QFileInfo file(url.path());
    if (file.suffix() == "svn") {
        return URL_Svn;
    }
    if (file.suffix() == "git") {
        return URL_Git;
    }
    return defaultValue;
}

NewProjectFromRemoteDialog::NewProjectFromRemoteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectFromRemoteDialog),
    priv(std::make_unique<NewProjectFromRemoteDialog::Priv_t>())
{
    ui->setupUi(this);
    priv->net = new QNetworkAccessManager(this);

    auto setIconFromUrl = [this](UrlType t)
    {
        auto icon = urlTypeIcons.value(t);
        auto iconPath = AppConfig::resourceImage({"mimetypes", icon});
        ui->downloadMethod->setIcon(QIcon(iconPath));
        priv->currentType = t;
    };
    auto x = makeMenuAction<UrlType>(this,
    {
        {tr("Git"), URL_Git},
        {tr("Svn"), URL_Svn},
        {tr("Archive"), URL_Archive},
    },
    setIconFromUrl);
    ui->downloadMethod->setMenu(x.first);
    ui->downloadMethod->menu()->actions().first()->setChecked(true);
    setIconFromUrl(URL_Git);
    auto updateFileName = [this]() {
        QUrl url{ui->editUrl->text()};
        auto basename = QFileInfo{url.fileName()}.baseName();
        if (url.host() == "github.com") {
            static const QRegularExpression re(R"(^\/([^\/]+)\/([^\/]+))");
            auto m = re.match(url.path());
            ui->editName->setText(m.hasMatch()? m.captured(2) : basename);
        } else {
            ui->editName->setText(basename);
        }
        ui->editName->setModified(false);
    };
    auto updateFinalPath = [this]() {
        ui->labelFinalPath->setText(QStringList{
                                        ui->editPath->text(),
                                        ui->editName->text(),
                                    }.join(QDir::separator()));
    };
    auto tryToDetectType = [this, actionMap = x.second, setIconFromUrl]() {
        static const QRegularExpression validUrlRe(R"(^((([A-Za-z]{3,9}:(?:\/\/)?)(?:[\-;:&=\+\$,\w]+@)?[A-Za-z0-9\.\-]+|(?:www\.|[\-;:&=\+\$,\w]+@)[A-Za-z0-9\.\-]+)((?:\/[\+~%\/\.\w\-_]*)?\??(?:[\-\+=&;%@\.\w_]*)#?(?:[\.\!\/\\\w]*))?))");
        auto url = ui->editUrl->text();
        ui->buttonDownload->setEnabled(validUrlRe.match(url).hasMatch());
        auto t = detectUrlType(url, URL_Archive);
        actionMap.value(t)->setChecked(true);
        setIconFromUrl(t);
    };
    auto selectProjectFolder = [this]() {
        auto d = ui->editPath->text();
        if (!QDir(d).exists())
            d = QDir::homePath();
        d = QFileDialog::getExistingDirectory(window(), tr("Select Folder"), d);
        if (!d.isEmpty())
            ui->editPath->setText(d);
    };

    ui->editPath->setText(AppConfig::instance().projectsPath());
    connect(ui->editUrl, &QLineEdit::textChanged, updateFileName);
    connect(ui->editUrl, &QLineEdit::textChanged, tryToDetectType);
    connect(ui->editName, &QLineEdit::textChanged, updateFinalPath);
    connect(ui->editPath, &QLineEdit::textChanged, updateFinalPath);
    connect(ui->openFolderButton, &QToolButton::clicked, selectProjectFolder);
    connect(ui->buttonDownload, &QAbstractButton::clicked, this, &NewProjectFromRemoteDialog::startDownload);
    connect(ui->buttonClose, &QAbstractButton::clicked, [this]() {
        if (priv->status == InProgress) {
            cancelAll();
        } else {
            reject();
        }
    });
}

NewProjectFromRemoteDialog::~NewProjectFromRemoteDialog()
{
    delete ui;
}

QString NewProjectFromRemoteDialog::projectPath() const
{
    return ui->labelFinalPath->text();
}

void NewProjectFromRemoteDialog::startDownload()
{
    if (priv->status == FinishOk) {
        accept();
        return;
    }
    if (priv->currentType == URL_Archive) {
        handleArchiveType();
        return;
    }
    auto captureStdout = [this](QProcess *p) {
        ConsoleInterceptor::writeMessageTo(ui->log, filterOut(p->readAllStandardOutput()), Qt::darkBlue);
    };
    auto captureStderr = [this](QProcess *p) {
        ConsoleInterceptor::writeMessageTo(ui->log, filterOut(p->readAllStandardError()), Qt::darkRed);
    };
    auto processEnd = [this](QProcess *p, int exitCode) {
        ConsoleInterceptor::writeMessageTo(ui->log, tr("\nFinished with %1\n").arg(exitCode));
        if (p->exitStatus() == QProcess::NormalExit) {
            setOperationInProgress(FinishOk);
            ui->buttonDownload->setText(tr("Try to open"));
        } else {
            setOperationInProgress(FinishError);
        }
    };
    auto processError = [this](QProcess *p, QProcess::ProcessError) {
        ConsoleInterceptor::writeMessageTo(ui->log, tr("\nERROR:  %1\n").arg(p->errorString()));
        setOperationInProgress(FinishError);
    };
    auto& p = ChildProcess::create(this)
            .setenv({ { "LANG", "C" }, { "LC_ALL", "C" } })
            .changeCWD(ui->editPath->text())
            .makeDeleteLater()
            .onReadyReadStdout(captureStdout)
            .onReadyReadStderr(captureStderr)
            .onFinished(processEnd)
            .onError(processError)
            .onStarted([this](QProcess*) { setOperationInProgress(InProgress); })
    ;
    connect(this, &NewProjectFromRemoteDialog::cancelRequests, &p, &QProcess::terminate);

    QVariantHash map = {
        { "url", ui->editUrl->text() },
        { "path", ui->labelFinalPath->text() },
    };
    auto tCmd = commandForType.value(priv->currentType);
    auto cmd = Mustache::renderTemplate(tCmd, map);

    ui->log->clear();
    ConsoleInterceptor::writeMessageTo(ui->log, tr("Starting: %1\n\n").arg(cmd));
    priv->status = InProgress;
    p.start(cmd, QProcess::Unbuffered | QProcess::ReadWrite);
}

void NewProjectFromRemoteDialog::cancelAll()
{
    emit cancelRequests();
}

static int percent(qint64 a, qint64 b)
{
    return (a * 1024 * 100) / (b * 1024);
}

void NewProjectFromRemoteDialog::handleArchiveType()
{
    auto urlText = ui->editUrl->text();
    auto ext = QFileInfo(urlText).completeSuffix();
    auto tcmd = commandForFileType.value(ext, {});
    if (tcmd.isEmpty()) {
        ConsoleInterceptor::writeMessageTo(ui->log, tr("Unknown file format: %1\n").arg(ext));
        return;
    }
    auto projectPath = ui->labelFinalPath->text();
    auto root = QDir(projectPath).root();
    if (!root.mkpath(projectPath)) {
        ConsoleInterceptor::writeMessageTo(ui->log, tr("Cannot create project directory %1: %2\n").arg(projectPath));
        return;
    }
    auto& p = ChildProcess::create(this)
            .changeCWD(projectPath)
            .mergeStdOutAndErr()
            // TODO
    ;
    auto cmd = Mustache::renderTemplate(tcmd,
    {
        { "url", ui->editUrl->text() },
        { "path", ui->labelFinalPath->text() },
    });
    p.start(cmd);
    ui->log->clear();
    setOperationInProgress(InProgress);
    ConsoleInterceptor::writeMessageTo(ui->log, tr("Start downloading %1...\n").arg(urlText));
    auto request = QNetworkRequest(QUrl(urlText));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    auto reply = priv->net->get(request);
    if (reply) {
        auto updateProgress = [this](qint64 r, qint64 t) {
            if (t == -1) {
                ui->progressBar->setRange(0, 0);
                ui->progressBar->setValue(0);
                ConsoleInterceptor::writeMessageTo(ui->log, tr("Downloading: %1 bytes\n").arg(r));
            } else if (t == 0) {
                ui->progressBar->setValue(0);
            } else {
                ui->progressBar->setValue(percent(r, t));
            }
        };
        auto onRedirect = [this](const QUrl& url) {
            ConsoleInterceptor::writeMessageTo(ui->log, tr("Redirect: %1\n").arg(url.toString()));
        };
        auto onError = [this, reply](QNetworkReply::NetworkError) {
            ConsoleInterceptor::writeMessageTo(ui->log, tr("ERROR: %1\n").arg(reply->errorString()));
            setOperationInProgress(FinishError);
            reply->deleteLater();
        };
        auto onFinish = [this, reply]() {
            setOperationInProgress(FinishOk);
            ConsoleInterceptor::writeMessageTo(ui->log, tr("\nFinished"));
            reply->deleteLater();
        };
        auto finalizeRequest = [this]() {
            ui->progressBar->setRange(0, 100);
            ui->progressBar->reset();
        };
        connect(reply, &QNetworkReply::downloadProgress, updateProgress);
        connect(reply, &QNetworkReply::redirected, onRedirect);
        connect(reply, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error), onError);
        connect(reply, &QNetworkReply::finished, onFinish);
        connect(reply, &QObject::destroyed, finalizeRequest);
    }
}

void NewProjectFromRemoteDialog::setOperationInProgress(ProcessStatus status)
{
    priv->status = status;
    switch (status) {
    case NotStarted:
        ui->buttonDownload->setEnabled(true);
        ui->buttonClose->setText(tr("Close"));
        break;
    case InProgress:
        ui->buttonDownload->setEnabled(false);
        ui->buttonClose->setText(tr("Cancel"));
        break;
    case FinishOk:
        ui->buttonDownload->setEnabled(true);
        ui->buttonDownload->setText(tr("Open Project"));
        ui->buttonClose->setText(tr("Close"));
        break;
    case FinishError:
        ui->buttonDownload->setEnabled(true);
        ui->buttonDownload->setText(tr("Retry"));
        setOperationInProgress(NotStarted);
        break;
    }
}

QString NewProjectFromRemoteDialog::filterOut(const QString &text)
{
    static const QRegularExpression re(R"(\b(\d+)\s*\%)");
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        auto p = m.captured(1).toInt();
        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(p);
    }
    return text;
}
