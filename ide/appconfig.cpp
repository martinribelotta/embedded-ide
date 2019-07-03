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

#include <QtDebug>

class AppConfig::Priv_t
{
public:
    QJsonObject global;
    QJsonObject local;
    QProcessEnvironment sysenv;
};

#define CFG_GLOBAL (priv->global)
#define CFG_LOCAL (priv->local)


static const QString BUNDLE_GLOBAL_PATH = ":/default-global.json";
static const QString BUNDLE_LOCAL_PATH = ":/default-local.json";

#ifdef Q_OS_WIN
static const QChar PATH_SEP = ';';
#else
static const QChar PATH_SEP = ':';
#endif


static const QJsonValue& valueOrDefault(const QJsonValue& v, const QJsonValue& d)
{
    return v.isUndefined()? d : v;
}

static QByteArray readEntireFile(const QString& path, const QByteArray& ifFail = QByteArray())
{
    QFile f(path);
    if (f.open(QFile::ReadOnly))
        return QTextStream(&f).readAll().toUtf8();
    else
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
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(readEntireFile(path), &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "error reading" << path << err.errorString();
    }
    return doc.object();
}

static QString globalConfigFilePath()
{
    static const auto name = "." + QApplication::instance()->applicationDirPath()
                                       .replace("/", "-")
                                       .replace("\\", "-")
                                       .replace(":", "") + ".json";
    return QDir::home().absoluteFilePath(name);
}

static QDir sharedDir()
{
    return QDir(QApplication::applicationDirPath()).absoluteFilePath(
#ifdef Q_OS_UNIX
        "../share/embedded-ide/"
#else
        "./"
#endif
    );
}

static const QString systemGlobalConfigPath() {
    return sharedDir().absoluteFilePath("embedded_ide-config.json");
}

static const QString systemLocalConfigPath() {
    return sharedDir().absoluteFilePath("embedded-ide.hardconf");
}

static const QString systemTranslationPath() {
    return sharedDir().absoluteFilePath("translations/");
}

static void addResourcesFont()
{
    for(const auto& fontPath: QDir(":/fonts/").entryInfoList({ "*.ttf" }))
        QFontDatabase::addApplicationFont(fontPath.absoluteFilePath());
}

static bool completeToLeft(QJsonObject& left, const QJsonObject& right)
{
    bool modify = false;
    for(const auto& k: right.keys()) {
        if (left.contains(k)) {
            auto obj = left[k].toObject();
            if (completeToLeft(obj, left[k].toObject())) {
                modify = true;
                left.insert(k, obj);
            }
        } else {
            modify = true;
            left.insert(k, right[k]);
        }
    }
    return modify;
}

AppConfig::AppConfig() : QObject(QApplication::instance()), priv(new Priv_t)
{
    priv->sysenv = QProcessEnvironment::systemEnvironment();
    addResourcesFont();

    adjustEnv();
    load();
    adjustEnv();
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
    if (!priv->sysenv.contains("HOME")) {
        auto homePath = QDir::home().absolutePath();
        auto homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (!homePaths.isEmpty())
            homePath = homePaths.first();
        qputenv("HOME", homePath.toLocal8Bit());
    }

    qputenv("APPLICATION_DIR_PATH", QApplication::applicationDirPath().toLocal8Bit());
    qputenv("APPLICATION_FILE_PATH", QApplication::applicationFilePath().toLocal8Bit());
    if (!CFG_LOCAL.isEmpty()) {
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
    auto extras = additionalPaths().join(PATH_SEP);
    auto path = QString("%1%2%3").arg(extras).arg(PATH_SEP).arg(old);
    qputenv("PATH", path.toLocal8Bit());
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

QIODevice *AppConfig::writeEntireTextFile(const QString& text, const QString& path)
{
    auto *f = new QFile(path, QApplication::instance());
    if (f->open(QFile::WriteOnly)) {
        f->write(text.toLocal8Bit());
    }
    f->deleteLater();
    return f;
}

QString AppConfig::workspacePath() const
{
    QJsonValue defaultPath = QDir::home().absoluteFilePath(".embedded_ide-workspace");
    return replaceWithEnv(valueOrDefault(CFG_GLOBAL.value("workspacePath"), defaultPath).toString());
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
    auto s = QString(":/images/%1/%2.%3").arg(style).arg(path).arg(ext);
    return s;
}

QString AppConfig::resourceImage(const QStringList &pathPart, const QString &ext)
{
    return AppConfig::resourceImage(pathPart.join(QDir::separator()), ext);
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
    auto vtools = CFG_LOCAL["externalTools"];
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
    auto history = CFG_LOCAL["history"].toArray();
    for(const auto e: history) {
        QFileInfo info(e.toString());
        if (info.exists() && !list.contains(info))
            list.append(info);
    }
    return list;
}

QStringList AppConfig::additionalPaths(bool raw) const
{
    QStringList paths;
    if (raw)
        for(const auto e: CFG_LOCAL.value("additionalPaths").toArray())
            paths.append(e.toString());
    else
        for(const auto e: CFG_LOCAL.value("additionalPaths").toArray())
            paths.append(replaceWithEnv(e.toString()));
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

QString AppConfig::editorFormatterExtra() const
{
    return CFG_LOCAL.value("editor").toObject().value("formatterExtra").toString();
}

bool AppConfig::editorDetectIdent() const
{
    return CFG_LOCAL.value("editor").toObject().value("detectIdent").toBool();
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

bool AppConfig::useDarkStyle() const
{
    return CFG_LOCAL.value("useDarkStyle").toBool();
}

QString AppConfig::language() const
{
    return CFG_LOCAL.value("lang").toString();
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

void AppConfig::load()
{
    auto bundleGlobalCfg = loadJson(BUNDLE_GLOBAL_PATH);
    auto systemGlobalCfg = loadJson(systemGlobalConfigPath());
    CFG_GLOBAL = loadJson(globalConfigFilePath());
    completeToLeft(systemGlobalCfg, bundleGlobalCfg);
    if (completeToLeft(CFG_GLOBAL, systemGlobalCfg))
        writeEntireFile(globalConfigFilePath(), QJsonDocument(CFG_GLOBAL).toJson());

    auto bundleLocalCfg = loadJson(BUNDLE_LOCAL_PATH);
    auto systemLocalCfg = loadJson(systemLocalConfigPath());
    CFG_LOCAL = loadJson(localConfigFilePath());
    completeToLeft(systemLocalCfg, bundleLocalCfg);
    if (completeToLeft(CFG_LOCAL, systemLocalCfg))
        writeEntireFile(localConfigFilePath(), QJsonDocument(CFG_LOCAL).toJson());

    projectsPath();
    templatesPath();

    QIcon::setThemeSearchPaths({
        ":/images/dark",
        ":/images/light"
    });
    QIcon::setThemeName(useDarkStyle()? "dark" : "light");

    emit configChanged(this);
}

void AppConfig::save()
{
    writeEntireFile(globalConfigFilePath(), QJsonDocument(CFG_GLOBAL).toJson());
    writeEntireFile(localConfigFilePath(), QJsonDocument(CFG_LOCAL).toJson());
    adjustEnv();
    emit configChanged(this);
}

void AppConfig::setWorkspacePath(const QString &path)
{
    CFG_GLOBAL.insert("workspacePath", path);
}

void AppConfig::setExternalTools(const QList<QPair<QString, QString> > &tools)
{
    QJsonArray a;
    for (const auto& it: tools)
        a.append(QJsonObject{ { it.first, it.second } });
    CFG_LOCAL.insert("externalTools", a);
}

void AppConfig::appendToRecentProjects(const QString &path)
{
    if (!path.startsWith(projectsPath())) {
        QJsonArray history = CFG_LOCAL["history"].toArray();
        if (!history.contains(path))
            history.append(path);
        CFG_LOCAL["history"] = history;
    }
}

void AppConfig::setAdditionalPaths(const QStringList &paths)
{
    QJsonArray array;
    for(const auto& p: paths)
        array.append(p);
    CFG_LOCAL.insert("additionalPaths", array);
}

void AppConfig::setTemplatesUrl(const QString &url)
{
    auto t = CFG_LOCAL["templates"].toObject();
    t.insert("url", url);
    CFG_LOCAL["templates"] = t;
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

void AppConfig::setEditorFormatterExtra(const QString &text)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed.insert("formatterExtra", text);
    CFG_LOCAL["editor"] = ed;
}

void AppConfig::setEditorDetectIdent(bool enable)
{
    auto ed = CFG_LOCAL["editor"].toObject();
    ed.insert("detectIdent", enable);
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

void AppConfig::setUseDarkStyle(bool use)
{
    CFG_LOCAL.insert("useDarkStyle", use);
}

void AppConfig::setLanguage(const QString &lang)
{
    CFG_LOCAL.insert("lang", lang);
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
        if (!QFileInfo(QDir(templatesPath()).filePath(k)).exists())
            o.remove(k);
    writeEntireFile(path, QJsonDocument(o).toJson());
}
