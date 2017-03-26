#include "appconfig.h"

#include "passwordpromtdialog.h"

#include <QSettings>
#include <QDir>

#define EDITOR_STYLE "editor/style"
#define EDITOR_FONT_SIZE "editor/font/size"
#define EDITOR_FONT_STYLE "editor/font/style"
#define EDITOR_SAVE_ON_ACTION "editor/saveOnAction"
#define EDITOR_TABS_TO_SPACES "editor/tabsToSpaces"
#define EDITOR_TAB_WIDTH "editor/tabWidth"
#define BUILD_DEFAULT_PROJECT_PATH "build/defaultprojectpath"
#define BUILD_TEMPLATE_PATH "build/templatepath"
#define BUILD_TEMPLATE_URL "build/templateurl"
#define BUILD_ADDITIONAL_PATHS "build/additional_path"
#define NETWORK_PROXY_TYPE "/network/proxy/type"
#define NETWORK_PROXY_HOST "/network/proxy/host"
#define NETWORK_PROXY_PORT "/network/proxy/port"
#define NETWORK_PROXY_CREDENTIALS "/network/proxy/credentials"
#define NETWORK_PROXY_USERNAME "/network/proxy/username"

struct AppConfigData {
    struct NetworkProxy {
        AppConfig::NetworkProxyType type;
        bool useCredentials;
        QString host;
        QString port;
        QString username;
        QString password;
    } networkProxy;

    QStringList buildAdditionalPaths;
    QString editorStyle;
    QString editorFontStyle;
    QString builDefaultProjectPath;
    QString builTemplatePath;
    QString builTemplateUrl;
    int editorFontSize;
    int editorTabWidth;
    bool editorSaveOnAction;
    bool editorTabsToSpaces;
};

AppConfigData* appData()
{
  static AppConfigData appData;
  return &appData;
}

AppConfig& AppConfig::mutableInstance()
{
  static AppConfig appConfig;
  return appConfig;
}

const QStringList &AppConfig::buildAdditionalPaths() const
{
  return appData()->buildAdditionalPaths;
}

const QString& AppConfig::editorStyle() const
{
  return appData()->editorStyle;
}

int AppConfig::editorFontSize() const
{
  return appData()->editorFontSize;
}

const QString& AppConfig::editorFontStyle() const
{
  return appData()->editorFontStyle;
}

bool AppConfig::editorSaveOnAction() const
{
  return appData()->editorSaveOnAction;
}

bool AppConfig::editorTabsToSpaces() const
{
  return appData()->editorTabsToSpaces;
}

int AppConfig::editorTabWidth() const
{
  return appData()->editorTabWidth;
}

const QString& AppConfig::builDefaultProjectPath() const
{
  return appData()->builDefaultProjectPath;
}

const QString &AppConfig::builTemplatePath() const
{
  return appData()->builTemplatePath;
}

const QString &AppConfig::builTemplateUrl() const
{
  return appData()->builTemplateUrl;
}

QString AppConfig::defaultApplicationResources() const
{
  return QDir::home().absoluteFilePath("embedded-ide-workspace");
}

QString AppConfig::defaultProjectPath()
{
  return QDir(defaultApplicationResources()).absoluteFilePath("projects");
}

QString AppConfig::defaultTemplatePath()
{
  return QDir(defaultApplicationResources()).absoluteFilePath("templates");
}

QString AppConfig::defaultTemplateUrl()
{
    return "https://api.github.com/repos/ciaa/EmbeddedIDE-templates/contents";
}

const QString& AppConfig::networkProxyHost() const
{
    return appData()->networkProxy.host;
}

QString AppConfig::networkProxyPort() const
{
    return appData()->networkProxy.port;
}

bool AppConfig::networkProxyUseCredentials() const
{
    return appData()->networkProxy.useCredentials;
}

AppConfig::NetworkProxyType AppConfig::networkProxyType() const
{
    return appData()->networkProxy.type;
}

const QString& AppConfig::networkProxyUsername() const
{
    return appData()->networkProxy.username;
}

const QString& AppConfig::networkProxyPassword() const
{
    return appData()->networkProxy.password;
}

AppConfig::AppConfig()
{
  load();
}

void AppConfig::load()
{
  QSettings s;
  this->setEditorStyle(
        s.value(EDITOR_STYLE, "Solarized-light").toString());
  this->setEditorFontSize(
        s.value(EDITOR_FONT_SIZE, 10).toInt());
  this->setEditorFontStyle(
        s.value(EDITOR_FONT_STYLE, "DejaVu Sans Mono").toString());
  this->setEditorSaveOnAction(
        s.value(EDITOR_SAVE_ON_ACTION, true).toBool());
  this->setEditorTabsToSpaces(
        s.value(EDITOR_TABS_TO_SPACES, true).toBool());
  this->setEditorTabWidth(
        s.value(EDITOR_TAB_WIDTH, 4).toInt());
  this->setBuilDefaultProjectPath(
        s.value(BUILD_DEFAULT_PROJECT_PATH, defaultProjectPath()).toString());
  this->setBuilTemplatePath(
        s.value(BUILD_TEMPLATE_PATH, defaultTemplatePath()).toString());
  this->setBuilTemplateUrl(
        s.value(BUILD_TEMPLATE_URL, defaultTemplateUrl()).toString());
  this->setBuildAdditionalPaths(
        s.value(BUILD_ADDITIONAL_PATHS).toStringList());
  this->setNetworkProxyType(
        static_cast<NetworkProxyType>(
          s.value(NETWORK_PROXY_TYPE, false).toInt()));
  this->setNetworkProxyHost(
        s.value(NETWORK_PROXY_HOST).toString());
  this->setNetworkProxyPort(
        s.value(NETWORK_PROXY_PORT).toString());
  this->setNetworkProxyUseCredentials(
        s.value(NETWORK_PROXY_CREDENTIALS).toBool());
  this->setNetworkProxyUsername(
        s.value(NETWORK_PROXY_USERNAME).toString());
  if (this->networkProxyType() == NetworkProxyType::Custom
      && this->networkProxyUseCredentials()) {
    PasswordPromtDialog paswd(
          PasswordPromtDialog::tr("Proxy require password"));
    if (paswd.exec() == QDialog::Accepted) {
      this->setNetworkProxyPassword(paswd.password());
    }
  }
  emit configChanged(this);
}

void AppConfig::save()
{
  QSettings s;
  s.setValue(EDITOR_STYLE, appData()->editorStyle);
  s.setValue(EDITOR_FONT_SIZE, appData()->editorFontSize);
  s.setValue(EDITOR_FONT_STYLE, appData()->editorFontStyle);
  s.setValue(EDITOR_SAVE_ON_ACTION, appData()->editorSaveOnAction);
  s.setValue(EDITOR_TABS_TO_SPACES, appData()->editorTabsToSpaces);
  s.setValue(EDITOR_TAB_WIDTH, appData()->editorTabWidth);
  s.setValue(BUILD_DEFAULT_PROJECT_PATH, appData()->builDefaultProjectPath);
  s.setValue(BUILD_TEMPLATE_PATH, appData()->builTemplatePath);
  s.setValue(BUILD_TEMPLATE_URL, appData()->builTemplateUrl);
  s.setValue(BUILD_ADDITIONAL_PATHS, appData()->buildAdditionalPaths);
  s.setValue(NETWORK_PROXY_TYPE, static_cast<int>(this->networkProxyType()));
  s.setValue(NETWORK_PROXY_HOST, this->networkProxyHost());
  s.setValue(NETWORK_PROXY_PORT, this->networkProxyPort());
  s.setValue(NETWORK_PROXY_CREDENTIALS, this->networkProxyUseCredentials());
  s.setValue(NETWORK_PROXY_USERNAME, this->networkProxyUsername());
  this->adjustPath();
  emit configChanged(this);
}

void AppConfig::setBuildAdditionalPaths(
    const QStringList& buildAdditionalPaths) const
{
  appData()->buildAdditionalPaths = buildAdditionalPaths;
}

void AppConfig::setEditorStyle(const QString &editorStyle)
{
  appData()->editorStyle = editorStyle;
}

void AppConfig::setEditorFontSize(int editorFontSize)
{
  appData()->editorFontSize = editorFontSize;
}

void AppConfig::setEditorFontStyle(const QString &editorFontStyle)
{
  appData()->editorFontStyle = editorFontStyle;
}

void AppConfig::setEditorSaveOnAction(bool editorSaveOnAction)
{
  appData()->editorSaveOnAction = editorSaveOnAction;
}

void AppConfig::setEditorTabsToSpaces(bool editorTabsToSpaces)
{
  appData()->editorTabsToSpaces = editorTabsToSpaces;
}

void AppConfig::setEditorTabWidth(int editorTabWidth)
{
  appData()->editorTabWidth = editorTabWidth;
}

void AppConfig::setBuilDefaultProjectPath(const QString &builDefaultProjectPath)
{
  appData()->builDefaultProjectPath = builDefaultProjectPath;
}

void AppConfig::setBuilTemplatePath(const QString &builTemplatePath)
{
  appData()->builTemplatePath = builTemplatePath;
}

void AppConfig::setBuilTemplateUrl(const QString &builTemplateUrl)
{
  appData()->builTemplateUrl = builTemplateUrl;
}

void AppConfig::setNetworkProxyHost(const QString& host)
{
    appData()->networkProxy.host = host;
}

void AppConfig::setNetworkProxyPort(QString port)
{
    appData()->networkProxy.port = port;
}

void AppConfig::setNetworkProxyUseCredentials(bool useCredentials)
{
    appData()->networkProxy.useCredentials = useCredentials;
}

void AppConfig::setNetworkProxyType(NetworkProxyType type)
{
    appData()->networkProxy.type = type;
}

void AppConfig::setNetworkProxyUsername(const QString& username)
{
    appData()->networkProxy.username = username;
}

void AppConfig::setNetworkProxyPassword(const QString& password)
{
    appData()->networkProxy.password = password;
}

void AppConfig::adjustPath()
{
    const QChar path_separator
#ifdef Q_OS_WIN
    (';')
#else
    (':')
#endif
    ;
    QString path = qgetenv("PATH");
    QStringList pathList = path.split(path_separator);
    QStringList additional = mutableInstance().buildAdditionalPaths();
#ifdef Q_OS_WIN
    additional.replaceInStrings("/", R"(\)");
#endif
    pathList = additional + pathList;
    path = pathList.join(path_separator);
    qputenv("PATH", path.toLocal8Bit());
}
