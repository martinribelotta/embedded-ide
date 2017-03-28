#include "loggerwidget.h"

#include <QProcess>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QRegularExpression>
#include <QUrlQuery>

#include <QtDebug>

struct LoggerWidget::priv_t {
    QProcess *proc;
    QTextBrowser *view;

    void decode(const QString &text, QColor color);

    void addText(const QString &text, QColor color) {
        QTextCursor c = view->textCursor();
        c.insertHtml(QString("<span style=\"color: %2\">%1</span>")
                     .arg(text)
                     .arg(color.name()));
        view->setTextCursor(c);
    }
};


static bool isFixedPitch(const QFont & font) {
    const QFontInfo fi(font);
    // qDebug() << fi.family() << fi.fixedPitch();
    return fi.fixedPitch();
}

const QFont monoFont() {
    QFont font("monospace");
    if (isFixedPitch(font))
        return font;
    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font))
        return font;
    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font))
        return font;
    font.setFamily("courier");
    if (isFixedPitch(font))
        return font;
    // qDebug() << font << "fallback";
    return font;
}

LoggerWidget::LoggerWidget(QWidget *parent) :
    QWidget(parent), d_ptr(new LoggerWidget::priv_t)
{
    QHBoxLayout *hlayout = new QHBoxLayout();
    QVBoxLayout *vlayout = new QVBoxLayout();
    QToolButton *clearConsole = new QToolButton(this);
    QToolButton *killProc = new QToolButton(this);
    QTextBrowser *view = new QTextBrowser(this);
    QProcess *proc = new QProcess(this);
    constexpr QSize iconSize(32, 32);

    clearConsole->setIcon(QIcon("://images/actions/edit-clear.svg"));
    killProc->setIcon(QIcon("://images/actions/media-playback-stop.svg"));
    clearConsole->setIconSize(iconSize);
    killProc->setIconSize(iconSize);
    killProc->setEnabled(false);
    view->setWordWrapMode(QTextOption::NoWrap);

    connect(clearConsole, &QToolButton::clicked, [this]() {
        clearText();
    });
    connect(killProc, &QToolButton::clicked, [this]() {
        QProcess *buildProc = d_ptr->proc;
        buildProc->terminate();
        if (!buildProc->waitForFinished(1000)) {
            d_ptr->addText("Killing process...", Qt::red);
            buildProc->kill();
        }
    });

    view->setOpenExternalLinks(false);
    view->setOpenLinks(false);
    view->document()->setDefaultFont(monoFont());
    connect(view, &QTextBrowser::anchorClicked, [this](const QUrl& url) {
        QUrlQuery q(url.query());
        int row = q.queryItemValue("x").toInt();
        int col = q.queryItemValue("y").toInt();
        emit openEditorIn(url.toLocalFile(), col, row);
    });

    hlayout->setContentsMargins(1, 1, 1, 1);
    hlayout->setSpacing(1);
    hlayout->addWidget(view);
    vlayout->addWidget(clearConsole);
    vlayout->addWidget(killProc);
    vlayout->addStretch(100);
    hlayout->addLayout(vlayout);
    setLayout(hlayout);

    LoggerWidget::priv_t *d = d_ptr;
    connect(proc, &QProcess::readyReadStandardOutput, [this] () {
        QByteArray data = d_ptr->proc->readAllStandardOutput();
        d_ptr->decode(QString::fromLocal8Bit(data), Qt::blue);
    });
    connect(proc, &QProcess::readyReadStandardError, [this] () {
        QByteArray data = d_ptr->proc->readAllStandardError();
        d_ptr->decode(QString::fromLocal8Bit(data), Qt::red);
    });
    connect(proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);
    });
    connect(proc,
 #if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        &QProcess::errorOccurred,
#else
        static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
#endif
                [this](QProcess::ProcessError error) {
        Q_UNUSED(error);
        d_ptr->addText(tr("Process ERROR: %1").arg(d_ptr->proc->errorString()), Qt::darkRed);
    });
    connect(proc, &QProcess::stateChanged, [this, killProc](QProcess::ProcessState state) {
        killProc->setEnabled(state != QProcess::NotRunning);
    });
    d->proc = proc;
    d->view = view;
}

LoggerWidget::~LoggerWidget()
{
    delete d_ptr;
}

LoggerWidget &LoggerWidget::setWorkingDir(const QString& dir)
{
    d_ptr->proc->setWorkingDirectory(dir);
    return *this;
}

LoggerWidget &LoggerWidget::addEnv(const QStringList &extraEnv)
{
    return setEnv(d_ptr->proc->environment() + extraEnv);
}

LoggerWidget &LoggerWidget::setEnv(const QStringList &env)
{
    d_ptr->proc->setEnvironment(env);
    return *this;
}

void LoggerWidget::clearText()
{
    d_ptr->view->clear();
}

void LoggerWidget::addText(const QString &text, const QColor& color)
{
    d_ptr->addText(text, color);
}

bool LoggerWidget::startProcess(const QString &cmd, const QStringList &args)
{
    LoggerWidget::priv_t *d = d_ptr;
    if (d->proc->state() != QProcess::NotRunning)
        return false;
    d->view->clear();
    d->proc->start(cmd, args);
    return true;
}

bool LoggerWidget::startProcess(const QString &command)
{
    LoggerWidget::priv_t *d = d_ptr;
    if (d->proc->state() != QProcess::NotRunning)
        return false;
    d->view->clear();
    d->proc->start(command);
    return true;
}

static QString mkUrl(const QString& p, const QString& x, const QString& y) {
    return QString("file:%1?x=%2&y=%3").arg(p).arg(x).arg(y.toInt() - 1);
}

static QString consoleMarkError(const QString& s) {
    QString str(s);
    QRegularExpression re(R"(^(.+?):(\d+):(\d+):(.+?):(.+?)$)");
    re.setPatternOptions(QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = re.globalMatch(s);
    while(it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        QString text = m.captured(0);
        QString path = m.captured(1);
        QString line = m.captured(2);
        QString col = m.captured(3);
        QString url = mkUrl(path, line, col);
        str.replace(text, QString("<a href=\"%1\">%2</a>").arg(url).arg(text));
    }
    return str;
}

static QString consoleToHtml(const QString& s) {
    return consoleMarkError(QString(s)
            .replace("\t", "&nbsp;")
            .replace(" ", "&nbsp;"))
            .replace("\r\n", "<br>")
            .replace("\n", "<br>");
}

void LoggerWidget::priv_t::decode(const QString &text, QColor color)
{
    addText(consoleToHtml(text.toHtmlEscaped()), color);
}
