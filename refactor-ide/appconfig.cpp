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
#include <QMetaEnum>
#include <QFontDatabase>

#include <QtDebug>

class AppConfig::Priv_t
{
public:
    QJsonObject global;
    QJsonObject local;
};

#define CFG_GLOBAL (priv->global)
#define CFG_LOCAL (priv->local)

const QJsonValue& valueOrDefault(const QJsonValue& v, const QJsonValue& d)
{
    return v.isUndefined()? d : v;
}

const QJsonObject& objectOrDefault(const QJsonObject& v, const QJsonObject& d)
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

static void addResourcesFont()
{
    auto fontDir = QDir(":/fonts/");
    auto fontList = fontDir.entryInfoList({ "*.ttf" });
    for(const auto& fontPath: fontList)
        QFontDatabase::addApplicationFont(fontPath.absoluteFilePath());
}

static bool completeToLeft(QJsonObject& a, const QJsonObject& b)
{
    bool hasReplace = false;
    for(const auto& k: b.keys()) {
        auto v = a.value(k);
        if (v.isUndefined() || v.isNull()) {
            v = b.value(k);
            hasReplace = true;
        }
        if (v.isObject()) {
            auto o = v.toObject();
            hasReplace |= completeToLeft(o, b.value(k).toObject());
            v = o;
        }
        a.insert(k, v);
    }
    return hasReplace;
}

AppConfig::AppConfig() : QObject(QApplication::instance()), priv(new Priv_t)
{
    // Not in adjustEnv due prevent infinite recursion
    QProcessEnvironment::systemEnvironment().insert("APPLICATION_DIR_PATH", QApplication::applicationDirPath());
    QProcessEnvironment::systemEnvironment().insert("APPLICATION_FILE_PATH", QApplication::applicationFilePath());

    addResourcesFont();

    load();
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

QByteArray AppConfig::readEntireTextFile(const QString &path)
{
    return readEntireFile(path);
}

QString AppConfig::workspacePath() const
{
    QJsonValue defaultPath = QDir::home().absoluteFilePath(".embedded_ide-workspace");
    return valueOrDefault(CFG_GLOBAL.value("workspacePath"), defaultPath).toString();
}

static const QString& ensureExist(const QString& d)
{
    if (!QDir(d).exists())
        QDir::root().mkpath(d);
    return d;
}

QString AppConfig::projectsPath() const { return ensureExist(QDir(workspacePath()).absoluteFilePath("projects")); }

QString AppConfig::templatesPath() const { return ensureExist(QDir(workspacePath()).absoluteFilePath("templates")); }

QString AppConfig::localConfigFilePath() const { return QDir(ensureExist(workspacePath())).absoluteFilePath("config.json"); }

QStringList AppConfig::additionalPaths() const
{
    QStringList paths;
    for(const auto& e: CFG_LOCAL.value("additionalPaths").toArray())
        paths.append(e.toString());
    return paths;
}

QString AppConfig::templatesUrl() const
{
    return CFG_LOCAL.value("templates").toObject().value("url").toString();
}

QString AppConfig::editorStyle() const
{
    return valueOrDefault(CFG_LOCAL.value("editor").toObject().value("style"), "Default").toString();
}

QFont AppConfig::editorFont() const
{
    auto ed = CFG_LOCAL.value("editor").toObject();
    auto f = ed.value("font").toObject();
    auto name = f.value("name").toString();
    auto size = f.value("size").toInt(-1);
    return QFont(name, size);
}

bool AppConfig::editorSaveOnAction() const
{
    return CFG_LOCAL.value("editor").toObject().value("saveOnAction").toBool();
}

bool AppConfig::editorTabsToSpaces() const
{
    return CFG_LOCAL.value("editor").toObject().value("tabsOnSpaces").toBool();
}

int AppConfig::editorTabWidth() const
{
    return CFG_LOCAL.value("editor").toObject().value("tabWidth").toInt();
}

QString AppConfig::editorFormatterStyle() const
{
    return CFG_LOCAL.value("editor").toObject().value("formatterStyle").toString();
}

QFont AppConfig::loggerFont() const
{
    auto ed = CFG_LOCAL.value("logger").toObject();
    auto f = ed.value("font").toObject();
    auto name = f.value("name").toString();
    auto size = f.value("size").toInt(-1);
    return QFont(name, size);
}

QString AppConfig::networkProxyHost() const
{
    return CFG_LOCAL.value("network").toObject().value("proxy").toObject().value("host").toString();
}

QString AppConfig::networkProxyPort() const
{
    return CFG_LOCAL.value("network").toObject().value("proxy").toObject().value("port").toString();
}

bool AppConfig::networkProxyUseCredentials() const
{
    return CFG_LOCAL.value("network").toObject().value("proxy").toObject().value("useCredentials").toBool();
}

AppConfig::NetworkProxyType AppConfig::networkProxyType() const
{
    auto type = CFG_LOCAL.value("network").toObject().value("proxy").toObject().value("type").toString();
    bool ok = false;
    auto t = NetworkProxyType(QMetaEnum::fromType<AppConfig::NetworkProxyType>().keyToValue(type.toLatin1().data(), &ok));
    return ok? t : NetworkProxyType::None;
}

QString AppConfig::networkProxyUsername() const
{
    return CFG_LOCAL.value("network").toObject().value("proxy").toObject().value("user").toString();
}

QString AppConfig::networkProxyPassword() const
{
    return CFG_LOCAL.value("network").toObject().value("proxy").toObject().value("pass").toString();
}

bool AppConfig::projectTemplatesAutoUpdate() const
{
    return CFG_LOCAL.value("templates").toObject().value("autoUpdate").toBool();
}

bool AppConfig::useDevelopMode() const
{
    return CFG_LOCAL.value("useDevelopMode").toBool();
}

void AppConfig::load()
{
    auto def = QJsonDocument::fromJson(replaceWithEnv(readEntireFile(DEFAULT_LOCAL_RES)).toLocal8Bit()).object();
    CFG_GLOBAL = QJsonDocument::fromJson(replaceWithEnv(readEntireFile(globalConfigFilePath(), readEntireFile(DEFAULT_GLOBAL_RES))).toLocal8Bit()).object();
    CFG_LOCAL = QJsonDocument::fromJson(replaceWithEnv(readEntireFile(localConfigFilePath())).toLocal8Bit()).object();

    if (completeToLeft(CFG_LOCAL, def))
        save();

    projectsPath();
    templatesPath();

    emit configChanged(this);
}

void AppConfig::save()
{
    writeEntireFile(globalConfigFilePath(), QJsonDocument(CFG_GLOBAL).toJson());
    writeEntireFile(localConfigFilePath(), QJsonDocument(CFG_LOCAL).toJson());
    emit configChanged(this);
}

void AppConfig::setWorkspacePath(const QString &path)
{
    CFG_GLOBAL.insert("workspacePath", path);
}

void AppConfig::setAdditionalPaths(const QStringList &paths)
{
    QJsonArray array;
    for(const auto& p: paths)
        array.append(p);
    CFG_LOCAL.insert("additionalPaths", array);
}

void AppConfig::setEditorStyle(const QString &name)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed.insert("style", name);
    CFG_LOCAL["editor"] = ed;
}

void AppConfig::setEditorFont(const QFont &f)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed["font"] = QJsonObject{
        { "name", f.family() },
        { "size", f.pointSize() }
    };
    CFG_LOCAL["editor"] = ed;
}

void AppConfig::setEditorSaveOnAction(bool enable)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed.insert("saveOnAction", enable);
    CFG_LOCAL["editor"] = ed;
}

void AppConfig::setEditorTabsToSpaces(bool enable)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed.insert("tabsOnSpaces", enable);
    CFG_LOCAL["editor"] = ed;
}

void AppConfig::setEditorTabWidth(int n)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed.insert("tabWidth", n);
    CFG_LOCAL["editor"] = ed;
}

void AppConfig::setEditorFormatterStyle(const QString &name)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed.insert("formatterStyle", name);
    CFG_LOCAL["editor"] = ed;
}

void AppConfig::setLoggerFont(const QFont &f)
{
    auto log = CFG_LOCAL["logger"].toObject();
    log.insert("font", QJsonObject{
                   { "name", f.family() },
                   { "size", f.pointSize() }
               });
    CFG_LOCAL["logger"] = log;
}

void AppConfig::setNetworkProxyHost(const QString &name)
{
    auto net = CFG_LOCAL["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("host", name);
    net["proxy"] = proxy;
    CFG_LOCAL["network"] = net;
}

void AppConfig::setNetworkProxyPort(const QString &port)
{
    auto net = CFG_LOCAL["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("port", port);
    net["proxy"] = proxy;
    CFG_LOCAL["network"] = net;
}

void AppConfig::setNetworkProxyUseCredentials(bool use)
{
    auto net = CFG_LOCAL["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("useCredentials", use);
    net["proxy"] = proxy;
    CFG_LOCAL["network"] = net;
}

void AppConfig::setNetworkProxyType(AppConfig::NetworkProxyType type)
{
    auto typeName = QString(QMetaEnum::fromType<AppConfig::NetworkProxyType>().valueToKey(int(type)));
    auto net = CFG_LOCAL["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("type", typeName);
    net["proxy"] = proxy;
    CFG_LOCAL["network"] = net;
}

void AppConfig::setNetworkProxyUsername(const QString &user)
{
    auto net = CFG_LOCAL["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("user", user);
    net["proxy"] = proxy;
    CFG_LOCAL["network"] = net;
}

void AppConfig::setNetworkProxyPassword(const QString &pass)
{
    auto net = CFG_LOCAL["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("pass", pass);
    net["proxy"] = proxy;
    CFG_LOCAL["network"] = net;
}

void AppConfig::setProjectTemplatesAutoUpdate(bool en)
{
    auto t = CFG_LOCAL["templates"].toObject();
    t.insert("autoUpdate", en);
    CFG_LOCAL["templates"] = t;
}

void AppConfig::setUseDevelopMode(bool use)
{
    CFG_LOCAL.insert("useDevelopMode", use);
}
