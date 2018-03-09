#include "appconfig.h"

#include <QApplication>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QProcessEnvironment>
#include <QFont>
#include <QFontInfo>

class AppConfig::Priv_t
{
public:
    QJsonObject global;
    QJsonObject local;
};

const QJsonValue& valueOrDefault(const QJsonValue& v, const QJsonValue& d)
{
    return v.isUndefined()? d : v;
}

const QJsonObject& valueOrDefault(const QJsonObject& v, const QJsonObject& d)
{
    return v.empty()? d : v;
}

static QByteArray readEntireFile(const QString& path, const QByteArray& ifFail = QByteArray())
{
    QFile f(path);
    return f.open(QFile::ReadOnly)? QTextStream(&f).readAll().toUtf8() : ifFail;
}

static bool writeEntireFile(const QString& path, const QByteArray& data)
{
    QFile f(path);
    if (!f.open(QFile::WriteOnly))
        return false;
    return f.write(data) == data.length();
}

static QString globalConfigFilePath() { return QDir::home().absoluteFilePath(".embedded_ide-config.json"); }

const QString DEFAULT_GLOBAL_RES = ":/default-global.json";
const QString DEFAULT_LOCAL_RES = ":/default-local.json";

AppConfig::AppConfig() : QObject(QApplication::instance()), priv(new Priv_t)
{
    // Not in adjustEnv due prevent infinite recursion
    QProcessEnvironment::systemEnvironment().insert("APPLICATION_DIR_PATH", QApplication::applicationDirPath());
    QProcessEnvironment::systemEnvironment().insert("APPLICATION_FILE_PATH", QApplication::applicationFilePath());
    load();
    if (!QFileInfo(globalConfigFilePath()).exists() || !QFileInfo(localConfigFilePath()).exists())
        save();
}

AppConfig::~AppConfig()
{
    delete priv;
}

AppConfig &AppConfig::instance()
{
    static AppConfig *singleton = nullptr;
    if (!singleton)
        singleton = new AppConfig;
    return *singleton;
}

void AppConfig::adjustEnv()
{
#ifdef Q_OS_WIN
    QChar PATH_SEP = ';';
#else
    QChar PATH_SEP = ':';
#endif
    auto env = QProcessEnvironment::systemEnvironment();
    auto old = env.value("PATH");
    auto extras = instance().additionalPaths().join(PATH_SEP);
    auto path = QString("%1%2%3").arg(extras).arg(PATH_SEP).arg(old);
    env.insert("PATH", path);
}

static bool isFixedPitch(const QFont& font) { return QFontInfo(font).fixedPitch(); }

QFont AppConfig::systemMonoFont() {
#if defined(Q_OS_WIN)
    QFont font("Consolas");
#elif defined(Q_OS_LINUX)
    QFont font("Monospace");
#elif defined(Q_OS_MAC)
    QFont font("Monaco");
#else
    QFont font("Courier New");
#endif

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
    return font;
}

QString AppConfig::replaceWithEnv(const QString &str)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString copy(str);
    for(const auto& k: env.keys())
        copy.replace(QString("${%1}").arg(k), env.value(k));
    return copy;
}

QString AppConfig::workspacePath() const
{
    QJsonValue defaultPath = QDir::home().absoluteFilePath(".embedded_ide-workspace");
    return valueOrDefault(priv->global.value("workspacePath"), defaultPath).toString();
}

QString AppConfig::projectsPath() const { return QDir(workspacePath()).absoluteFilePath("projects"); }

QString AppConfig::templatesPath() const { return QDir(workspacePath()).absoluteFilePath("templates"); }

QString AppConfig::localConfigFilePath() const { return QDir(workspacePath()).absoluteFilePath("config.json"); }

QStringList AppConfig::additionalPaths() const
{
    QStringList paths;
    for(const auto& e: priv->local.value("additionalPaths").toArray())
        paths.append(e.toString());
    return paths;
}

QString AppConfig::editorStyle() const
{

}

QFont AppConfig::editorFont() const
{

}

bool AppConfig::editorSaveOnAction() const
{

}

bool AppConfig::editorTabsToSpaces() const
{

}

int AppConfig::editorTabWidth() const
{

}

QString AppConfig::editorFormatterStyle() const
{

}

QFont AppConfig::loggerFont() const
{

}

QString AppConfig::networkProxyHost() const
{

}

QString AppConfig::networkProxyPort() const
{

}

bool AppConfig::networkProxyUseCredentials() const
{

}

AppConfig::NetworkProxyType AppConfig::networkProxyType() const
{

}

QString AppConfig::networkProxyUsername() const
{

}

QString AppConfig::networkProxyPassword() const
{

}

bool AppConfig::projectTemplatesAutoUpdate() const
{

}

bool AppConfig::useDevelopMode() const
{

}

void AppConfig::load()
{
    priv->global = QJsonDocument::fromJson(replaceWithEnv(readEntireFile(globalConfigFilePath(), readEntireFile(DEFAULT_GLOBAL_RES))).toLocal8Bit()).object();
    priv->local = QJsonDocument::fromJson(replaceWithEnv(readEntireFile(localConfigFilePath(), readEntireFile(DEFAULT_LOCAL_RES))).toLocal8Bit()).object();
}

void AppConfig::save()
{
    writeEntireFile(globalConfigFilePath(), QJsonDocument(priv->global).toJson());
    writeEntireFile(localConfigFilePath(), QJsonDocument(priv->local).toJson());
}

void AppConfig::setWorkspacePath(const QString &path)
{

}

void AppConfig::setAdditionalPaths(const QStringList &paths)
{

}

void AppConfig::setEditorStyle(const QString &name)
{

}

void AppConfig::setEditorFont(const QFont &f)
{

}

void AppConfig::setEditorSaveOnAction(bool enable)
{

}

void AppConfig::setEditorTabsToSpaces(bool enable)
{

}

void AppConfig::setEditorTabWidth(int n)
{

}

void AppConfig::setEditorFormatterStyle(const QString &name)
{

}

void AppConfig::setLoggerFont(const QFont &f)
{

}

void AppConfig::networkProxyHost(const QString &name)
{

}

void AppConfig::setNetworkProxyPort(const QString &port)
{

}

void AppConfig::setNetworkProxyUseCredentials(bool use)
{

}

void AppConfig::setNetworkProxyType(AppConfig::NetworkProxyType type)
{

}

void AppConfig::setNetworkProxyUsername(const QString &user)
{

}

void AppConfig::setNetworkProxyPassword(const QString &pass)
{

}

void AppConfig::setProjectTemplatesAutoUpdate(bool en)
{

}

void AppConfig::setUseDevelopMode(bool use)
{

}
