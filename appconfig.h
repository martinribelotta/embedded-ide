#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>

#include "configdialog.h"
#include "dialogconfigworkspace.h"

class AppConfig : public QObject
{
    Q_OBJECT
    // WARNING(denisacostaq@gmail.com):
    // These classes can access to the private constructor, but do not create
    // an instance through this, use mutableInstance instead.
    friend ConfigDialog;
    friend DialogConfigWorkspace;
  public:
    enum class NetworkProxyType {
      None, System, Custom
    };

    const QStringList& buildAdditionalPaths() const;
    const QString& editorStyle() const;
    int editorFontSize() const;
    const QString& editorFontStyle() const;
    int loggerFontSize() const;
    const QString &loggerFontStyle() const;
    QFont loggerFont() const {
        QFont font;
        font.setPixelSize(loggerFontSize());
        font.setFamily(loggerFontStyle());
        return font;
    }
    bool editorSaveOnAction() const;
    bool editorTabsToSpaces() const;
    int editorTabWidth() const;
    const QString& builDefaultProjectPath() const;
    const QString& builTemplatePath() const;
    const QString& builTemplateUrl() const;
    QString defaultApplicationResources() const;
    QString defaultProjectPath();
    QString defaultTemplatePath();
    QString defaultTemplateUrl();
    const QString& networkProxyHost() const;
    QString networkProxyPort() const;
    bool networkProxyUseCredentials() const;
    NetworkProxyType networkProxyType() const;
    const QString& networkProxyUsername() const;
    const QString& networkProxyPassword() const;
    bool projectTmplatesAutoUpdate() const;

    void adjustPath();

    static AppConfig& mutableInstance();

  signals:
    void configChanged(AppConfig*);
  private:
    AppConfig();
    void load();
    void save();

    void setBuildAdditionalPaths(const QStringList& buildAdditionalPaths) const;
    void setEditorStyle(const QString& editorStyle);
    void setLoggerFontSize(int loggerFontSize);
    void setLoggerFontStyle(const QString& loggerFontStyle);
    void setEditorFontSize(int editorFontSize);
    void setEditorFontStyle(const QString& editorFontStyle);
    void setEditorSaveOnAction(bool editorSaveOnAction);
    void setEditorTabsToSpaces(bool editorTabsToSpaces);
    void setEditorTabWidth(int editorTabWidth);
    void setBuilDefaultProjectPath(const QString& builDefaultProjectPath);
    void setBuilTemplatePath(const QString& builTemplatePath);
    void setBuilTemplateUrl(const QString& builTemplateUrl);
    void setNetworkProxyHost(const QString& host);
    void setNetworkProxyPort(QString host);
    void setNetworkProxyUseCredentials(bool useCredentials);
    void setNetworkProxyType(NetworkProxyType type);
    void setNetworkProxyUsername(const QString& username);
    void setNetworkProxyPassword(const QString& password);
    void setProjectTmplatesAutoUpdate(bool automatic);
};

#endif // APPCONFIG_H
