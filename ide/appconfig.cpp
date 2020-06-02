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
#include <QSaveFile>
#include <QStandardPaths>
#include <QToolButton>
#include <QDateTime>
#include <QOperatingSystemVersion>

#include <QtDebug>

class AppConfig::Priv_t
{
public:
    QJsonObject global;
    QJsonObject local;
    QProcessEnvironment sysenv;
};

static const QString BUNDLE_GLOBAL_PATH = ":/default-global.json";
static const QString BUNDLE_LOCAL_PATH = ":/default-local.json";

static const QJsonValue& valueOrDefault(const QJsonValue& v, const QJsonValue& d)
{
    return v.isUndefined()? d : v;
}

static QByteArray readEntireFile(const QString& path, const QByteArray& ifFail = QByteArray())
{
    QFile f(path);
    if (f.open(QFile::ReadOnly))
        return QTextStream(&f).readAll().toUtf8();
    return ifFail;
}

static bool writeEntireFile(const QString& path, const QByteArray& data)
{
    QSaveFile f(path);
    if (!f.open(QFile::WriteOnly))
        return false;
    f.write(data);
    return f.commit();
}

static QJsonObject loadJson(const QString& path)
{
    QJsonParseError err{};
    if (!QFileInfo{path}.exists())
        return {};
    auto doc = QJsonDocument::fromJson(readEntireFile(path), &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "error reading" << path << err.errorString();
    }
    return doc.object();
}

static bool isAppImage()
{
    return !qgetenv("APPIMAGE").isEmpty();
}

static bool isWindows()
{
    return QOperatingSystemVersion::current().type() == QOperatingSystemVersion::Windows;
}

static QString appFilePath()
{
    return isAppImage()? qgetenv("APPIMAGE") : QApplication::applicationFilePath();
}

static QString appDirPath()
{
    return isAppImage()? QFileInfo(qgetenv("APPIMAGE")).absolutePath() : QApplication::applicationDirPath();
}

static QString globalConfigFilePath()
{
    auto name = "." + appDirPath().replace("/", "-")
                                  .replace("\\", "-")
                                  .replace(":", "") + ".json";
    return QDir::home().absoluteFilePath(name);
}

static QDir sharedDir()
{
    auto sharedDirPath = (isAppImage() || isWindows())? "./" : "../share/embedded-ide";
    return QDir(appDirPath()).absoluteFilePath(sharedDirPath);
}

static QString systemGlobalConfigPath()
{
    return sharedDir().absoluteFilePath("embedded_ide-config.json");
}

static QString systemLocalConfigPath()
{
    return sharedDir().absoluteFilePath("embedded-ide.hardconf");
}

static QString systemTranslationPath()
{
    return sharedDir().absoluteFilePath("translations/");
}

static void addResourcesFont()
{
    for(const auto& fontPath: QDir(":/fonts/").entryInfoList({ "*.ttf" }))
        QFontDatabase::addApplicationFont(fontPath.absoluteFilePath());
}

AppConfig::AppConfig() : QObject(QApplication::instance()), priv(std::make_unique<Priv_t>())
{
    priv->sysenv = QProcessEnvironment::systemEnvironment();
    addResourcesFont();
    adjustEnv();
    load();
    adjustEnv();
}

AppConfig::~AppConfig()
{
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
    if (!priv->sysenv.contains("HOME")) {
        auto homePath = QDir::home().absolutePath();
        auto homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (!homePaths.isEmpty())
            homePath = homePaths.first();
        qputenv("HOME", homePath.toLocal8Bit());
    }

    qputenv("APPLICATION_DIR_PATH", appDirPath().toLocal8Bit());
    qputenv("APPLICATION_FILE_PATH", appFilePath().toLocal8Bit());
    if (!priv->local.isEmpty()) {
        qputenv("WORKSPACE_PATH", workspacePath().toLocal8Bit());
        qputenv("WORKSPACE_PROJECT_PATH", projectsPath().toLocal8Bit());
        qputenv("WORKSPACE_TEMPLATE_PATH", templatesPath().toLocal8Bit());
        qputenv("WORKSPACE_CONFIG_FILE", localConfigFilePath().toLocal8Bit());
    } else {
        qputenv("WORKSPACE_PATH", "");
        qputenv("WORKSPACE_PROJECT_PATH", "");
        qputenv("WORKSPACE_TEMPLATE_PATH", "");
        qputenv("WORKSPACE_CONFIG_FILE", "");
    }

    auto old = priv->sysenv.value("PATH");
    auto separator = isWindows()? ";" : ":";
    auto extras = replaceWithEnv(additionalPaths().join(separator));
    auto path = QString("%1%2%3").arg(extras).arg(separator).arg(old);
    qputenv("PATH", path.toLocal8Bit());

    auto extraEnv = additionalEnv();
    for (auto it = extraEnv.constBegin(); it != extraEnv.constEnd(); ++it) {
        auto key = it.key().toLocal8Bit();
        auto val = replaceWithEnv(it.value()).toLocal8Bit();
        qputenv(key.constData(), val);
    }
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
    return replaceWithEnv(valueOrDefault((priv->global).value("workspacePath"), defaultPath).toString());
}

const QString& AppConfig::ensureExist(const QString& d)
{
    if (!QDir(d).exists())
        QDir::root().mkpath(d);
    return d;
}

QStringList AppConfig::langList()
{
    QStringList langs;
    for(const auto& p: langPaths())
        for(const auto& l: QDir(p).entryInfoList({ "*.qm" }))
            langs += l.baseName();
    return langs;
}

QStringList AppConfig::langPaths()
{
    return { systemTranslationPath(), ":/i18n/" };
}

QString AppConfig::resourceImage(const QString &path, const QString &ext)
{
    auto style = instance().useDarkStyle()? "dark" : "light";
    auto s = QString(":/images/%1/%2.%3").arg(style, path, ext);
    return s;
}

QString AppConfig::resourceImage(const QStringList &pathPart, const QString &ext)
{
    return AppConfig::resourceImage(pathPart.join(QDir::separator()), ext);
}

void AppConfig::fixIconTheme(QWidget *w)
{
        Q_UNUSED(w)
//    for (auto *b: w->findChildren<QAbstractButton*>()) {
//        auto iconName = b->icon().name();
//        if (!iconName.isEmpty()) {
//            auto resPath = resourceImage({ "actions", iconName });
//            b->setIcon(QIcon(resPath));
//            qDebug() << b->objectName() << "change" << iconName << "for" << resPath;
//        } else {
//            qDebug() << "button" << b->objectName() << "no icon";
//        }
//    }
}

QString AppConfig::projectsPath() const
{
    return ensureExist(QDir(workspacePath()).absoluteFilePath("projects"));
}

QString AppConfig::templatesPath() const
{
    return ensureExist(QDir(workspacePath()).absoluteFilePath("templates"));
}

QString AppConfig::localConfigFilePath() const
{
    return QDir(ensureExist(workspacePath())).absoluteFilePath("config.json");
}

QList<QPair<QString, QString> > AppConfig::externalTools() const
{
    QList<QPair<QString, QString> > map;
    auto vtools = priv->local["externalTools"];
    if (vtools.isObject()) {
        auto tools = vtools.toObject();
        for (const auto& k: tools.keys())
            map.append({ k, tools.value(k).toString() });
    } else if (vtools.isArray()) {
        for (const auto v: vtools.toArray()) {
            auto o = v.toObject();
            auto ks = o.keys();
            if (!ks.first().isEmpty()) {
                auto k = ks.first();
                map.append({ k, o.value(k).toString() });
            }
        }
    }
    return map;
}

QFileInfoList AppConfig::recentProjects() const
{
    QFileInfoList list;
    for(const auto& e: QDir(projectsPath()).entryInfoList(QDir::Dirs)) {
        QFileInfo info(QDir(e.absoluteFilePath()).absoluteFilePath("Makefile"));
        if (info.isFile())
            list.append(info);
    }
    auto history = priv->local["history"].toArray();
    for(const auto e: history) {
        QFileInfo info(e.toString());
        if (info.exists() && !list.contains(info))
            list.append(info);
    }
    return list;
}

QStringList AppConfig::additionalPaths() const
{
    QStringList paths;
    for(const auto e: priv->local.value("additionalPaths").toArray())
        paths.append(e.toString());
    return paths;
}

static void objectToMap(QMap<QString, QString>& map, const QJsonObject& obj)
{
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
        map.insert(it.key(), it.value().toString());
}

QMap<QString, QString> AppConfig::additionalEnv() const
{
    QMap<QString, QString> map;
    auto env = priv->local.value("additionalEnv");
    qDebug() << env.toString();
    if (env.isArray()) {
        for (const auto& e: env.toArray())
            objectToMap(map, e.toObject());
    } else {
        objectToMap(map, env.toObject());
    }
    return map;
}

QString AppConfig::templatesUrl() const
{
    return priv->local.value("templates").toObject().value("url").toString();
}

QString AppConfig::editorStyle() const
{
    return valueOrDefault(priv->local.value("editor").toObject().value("style"), "Default").toString();
}

QFont AppConfig::editorFont() const
{
    auto ed = priv->local.value("editor").toObject();
    auto f = ed.value("font").toObject();
    auto name = f.value("name").toString();
    auto size = f.value("size").toInt(-1);
    return QFont(name, size);
}

bool AppConfig::editorSaveOnAction() const
{
    return priv->local.value("editor").toObject().value("saveOnAction").toBool();
}

bool AppConfig::editorTabsToSpaces() const
{
    return priv->local.value("editor").toObject().value("tabsOnSpaces").toBool();
}

int AppConfig::editorTabWidth() const
{
    return priv->local.value("editor").toObject().value("tabWidth").toInt();
}

bool AppConfig::editorShowSpaces() const
{
    return priv->local.value("editor").toObject().value("showSpaces").toBool();
}

QString AppConfig::editorFormatterStyle() const
{
    return priv->local.value("editor").toObject().value("formatterStyle").toString();
}

QString AppConfig::editorFormatterExtra() const
{
    return priv->local.value("editor").toObject().value("formatterExtra").toString();
}

bool AppConfig::editorDetectIdent() const
{
    return priv->local.value("editor").toObject().value("detectIdent").toBool();
}

QFont AppConfig::loggerFont() const
{
    auto ed = priv->local.value("logger").toObject();
    auto f = ed.value("font").toObject();
    auto name = f.value("name").toString();
    auto size = f.value("size").toInt(-1);
    return QFont(name, size);
}

QString AppConfig::networkProxyHost() const
{
    return priv->local.value("network").toObject().value("proxy").toObject().value("host").toString();
}

QString AppConfig::networkProxyPort() const
{
    return priv->local.value("network").toObject().value("proxy").toObject().value("port").toString();
}

bool AppConfig::networkProxyUseCredentials() const
{
    return priv->local.value("network").toObject().value("proxy").toObject().value("useCredentials").toBool();
}

AppConfig::NetworkProxyType AppConfig::networkProxyType() const
{
    auto type = priv->local.value("network").toObject().value("proxy").toObject().value("type").toString();
    bool ok = false;
    auto t = NetworkProxyType(QMetaEnum::fromType<AppConfig::NetworkProxyType>().keyToValue(type.toLatin1().data(), &ok));
    return ok? t : NetworkProxyType::None;
}

QString AppConfig::networkProxyUsername() const
{
    return priv->local.value("network").toObject().value("proxy").toObject().value("user").toString();
}

QString AppConfig::networkProxyPassword() const
{
    return priv->local.value("network").toObject().value("proxy").toObject().value("pass").toString();
}

bool AppConfig::projectTemplatesAutoUpdate() const
{
    return priv->local.value("templates").toObject().value("autoUpdate").toBool();
}

bool AppConfig::useDevelopMode() const
{
    return priv->local.value("useDevelopMode").toBool();
}

bool AppConfig::useDarkStyle() const
{
    return priv->local.value("useDarkStyle").toBool();
}

QString AppConfig::language() const
{
    return priv->local.value("lang").toString();
}

int AppConfig::numberOfJobs() const
{
    return priv->local.value("numberOfJobs").toInt(1);
}

bool AppConfig::numberOfJobsOptimal() const
{
    return priv->local.value("numberOfJobsOptimal").toBool(false);
}

QByteArray AppConfig::fileHash(const QString &filename)
{
    auto path = QDir(workspacePath()).filePath("hashes.json");
    auto o = QJsonDocument::fromJson(readEntireTextFile(path)).object();
    auto v = o.value(QFileInfo(filename).fileName());
    if (v.isUndefined())
        return QByteArray();
    return QByteArray::fromHex(v.toString().toLatin1());
}

static QString templateGlobalConfigPath()
{
    QFileInfo sysGlobalPath{systemGlobalConfigPath()};
    return sysGlobalPath.exists()? sysGlobalPath.absoluteFilePath() : BUNDLE_GLOBAL_PATH;
}

static QString templateLocalConfigPath()
{
    QFileInfo sysLocalPath{systemLocalConfigPath()};
    return sysLocalPath.exists()? sysLocalPath.absoluteFilePath() : BUNDLE_LOCAL_PATH;
}

void AppConfig::load()
{
    QFileInfo globalCfgInfo{globalConfigFilePath()};
    if (!globalCfgInfo.exists())
        QFile::copy(templateGlobalConfigPath(), globalCfgInfo.absoluteFilePath());
    priv->global = loadJson(globalCfgInfo.absoluteFilePath());

    QFileInfo localCfgInfo{localConfigFilePath()};
    if (!localCfgInfo.exists())
        QFile::copy(templateLocalConfigPath(), localCfgInfo.absoluteFilePath());
    priv->local = loadJson(localCfgInfo.absoluteFilePath());

    projectsPath();
    templatesPath();

    emit configChanged(this);
}

void AppConfig::save()
{
    writeEntireFile(globalConfigFilePath(), QJsonDocument((priv->global)).toJson());
    writeEntireFile(localConfigFilePath(), QJsonDocument(priv->local).toJson());
    adjustEnv();
    emit configChanged(this);
}

void AppConfig::setWorkspacePath(const QString &path)
{
    (priv->global).insert("workspacePath", path);
}

void AppConfig::setExternalTools(const QList<QPair<QString, QString> > &tools)
{
    QJsonArray a;
    for (const auto& it: tools)
        a.append(QJsonObject{ { it.first, it.second } });
    priv->local.insert("externalTools", a);
}

void AppConfig::appendToRecentProjects(const QString &path)
{
    if (!path.startsWith(projectsPath())) {
        QJsonArray history = priv->local["history"].toArray();
        if (!history.contains(path))
            history.append(path);
        priv->local["history"] = history;
    }
}

void AppConfig::setAdditionalPaths(const QStringList &paths)
{
    QJsonArray array;
    for(const auto& p: paths)
        array.append(p);
    priv->local.insert("additionalPaths", array);
}

void AppConfig::setAdditionalEnv(const QMap<QString, QString> &env)
{
    QJsonArray array;
    for (auto it= env.constBegin(); it!=env.constEnd(); ++it)
        array.append(QJsonObject{{ it.key(), it.value() }});
    priv->local.insert("additionalEnv", array);
}

void AppConfig::setTemplatesUrl(const QString &url)
{
    auto t = priv->local["templates"].toObject();
    t.insert("url", url);
    priv->local["templates"] = t;
}

void AppConfig::setEditorStyle(const QString &name)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("style", name);
    priv->local["editor"] = ed;
}

void AppConfig::setEditorFont(const QFont &f)
{
    auto ed = priv->local["editor"].toObject();
    ed["font"] = QJsonObject{
        { "name", f.family() },
        { "size", f.pointSize() }
    };
    priv->local["editor"] = ed;
}

void AppConfig::setEditorSaveOnAction(bool enable)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("saveOnAction", enable);
    priv->local["editor"] = ed;
}

void AppConfig::setEditorTabsToSpaces(bool enable)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("tabsOnSpaces", enable);
    priv->local["editor"] = ed;
}

void AppConfig::setEditorTabWidth(int n)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("tabWidth", n);
    priv->local["editor"] = ed;
}

void AppConfig::setEditorShowSpaces(bool show)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("showSpaces", show);
    priv->local["editor"] = ed;
}

void AppConfig::setEditorFormatterStyle(const QString &name)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("formatterStyle", name);
    priv->local["editor"] = ed;
}

void AppConfig::setEditorFormatterExtra(const QString &text)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("formatterExtra", text);
    priv->local["editor"] = ed;
}

void AppConfig::setEditorDetectIdent(bool enable)
{
    auto ed = priv->local["editor"].toObject();
    ed.insert("detectIdent", enable);
    priv->local["editor"] = ed;
}

void AppConfig::setLoggerFont(const QFont &f)
{
    auto log = priv->local["logger"].toObject();
    log.insert("font", QJsonObject{
                   { "name", f.family() },
                   { "size", f.pointSize() }
               });
    priv->local["logger"] = log;
}

void AppConfig::setNetworkProxyHost(const QString &name)
{
    auto net = priv->local["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("host", name);
    net["proxy"] = proxy;
    priv->local["network"] = net;
}

void AppConfig::setNetworkProxyPort(const QString &port)
{
    auto net = priv->local["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("port", port);
    net["proxy"] = proxy;
    priv->local["network"] = net;
}

void AppConfig::setNetworkProxyUseCredentials(bool use)
{
    auto net = priv->local["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("useCredentials", use);
    net["proxy"] = proxy;
    priv->local["network"] = net;
}

void AppConfig::setNetworkProxyType(AppConfig::NetworkProxyType type)
{
    auto typeName = QString(QMetaEnum::fromType<AppConfig::NetworkProxyType>().valueToKey(int(type)));
    auto net = priv->local["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("type", typeName);
    net["proxy"] = proxy;
    priv->local["network"] = net;
}

void AppConfig::setNetworkProxyUsername(const QString &user)
{
    auto net = priv->local["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("user", user);
    net["proxy"] = proxy;
    priv->local["network"] = net;
}

void AppConfig::setNetworkProxyPassword(const QString &pass)
{
    auto net = priv->local["network"].toObject();
    auto proxy = net.value("proxy").toObject();
    proxy.insert("pass", pass);
    net["proxy"] = proxy;
    priv->local["network"] = net;
}

void AppConfig::setProjectTemplatesAutoUpdate(bool en)
{
    auto t = priv->local["templates"].toObject();
    t.insert("autoUpdate", en);
    priv->local["templates"] = t;
}

void AppConfig::setUseDevelopMode(bool use)
{
    priv->local.insert("useDevelopMode", use);
}

void AppConfig::setUseDarkStyle(bool use)
{
    priv->local.insert("useDarkStyle", use);
}

void AppConfig::setLanguage(const QString &lang)
{
    priv->local.insert("lang", lang);
}

void AppConfig::setNumberOfJobs(int n)
{
    priv->local.insert("numberOfJobs", n);
}

void AppConfig::setNumberOfJobsOptimal(bool en)
{
    priv->local.insert("numberOfJobsOptimal", en);
}

void AppConfig::addHash(const QString &filename, const QByteArray &hash)
{
    auto path = QDir(workspacePath()).filePath("hashes.json");
    auto o = QJsonDocument::fromJson(readEntireTextFile(path)).object();
    o.insert(QFileInfo(filename).fileName(), QString(hash.toHex()));
    writeEntireFile(path, QJsonDocument(o).toJson());
}

void AppConfig::purgeHash()
{
    auto path = QDir(workspacePath()).filePath("hashes.json");
    auto o = QJsonDocument::fromJson(readEntireTextFile(path)).object();
    for(auto& k: o.keys())
        if (!QFileInfo::exists(QDir(templatesPath()).filePath(k)))
            o.remove(k);
    writeEntireFile(path, QJsonDocument(o).toJson());
}
