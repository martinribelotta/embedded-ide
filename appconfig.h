#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <functional>

#include "configdialog.h"
#include "dialogconfigworkspace.h"

int main(int argc, char** argv);

class AppConfig : public QObject
{
    Q_OBJECT
    // WARNING(denisacostaq@gmail.com):
    // These classes can access to the private constructor, but do not create
    // an instance through this, use mutableInstance instead.
    friend class ConfigDialog;
    friend class DialogConfigWorkspace;
    friend int ::main(int argc, char** argv);
public:
    enum class NetworkProxyType {
        None, System, Custom
    };

    const QString filterTextWithVariables(const QString& text) const;
    const QHash<QString, QString> getVariableMap() const;

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
    QString editorFormatterStyle() const;
    const QString& buildDefaultProjectPath() const;
    const QString& buildTemplatePath() const;
    const QString& buildTemplateUrl() const;
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
    void adjustPath(const QStringList& paths);

    static AppConfig& mutableInstance();

signals:
    void configChanged(AppConfig*);

private:
    AppConfig();
    void load();
    void save();

    void addFilterTextVariable(const QString& key, std::function<QString ()> func);
    void setBuildAdditionalPaths(const QStringList& buildAdditionalPaths) const;
    void setEditorStyle(const QString& editorStyle);
    void setLoggerFontSize(int loggerFontSize);
    void setLoggerFontStyle(const QString& loggerFontStyle);
    void setEditorFontSize(int editorFontSize);
    void setEditorFontStyle(const QString& editorFontStyle);
    void setEditorSaveOnAction(bool editorSaveOnAction);
    void setEditorTabsToSpaces(bool editorTabsToSpaces);
    void setEditorTabWidth(int editorTabWidth);
    void setEditorFormatterStyle(const QString& style);
    void setWorkspacePath(const QString& workspacePath);
    void setBuildDefaultProjectPath(const QString& buildDefaultProjectPath);
    void setBuildTemplatePath(const QString& buildTemplatePath);
    void setBuildTemplateUrl(const QString& buildTemplateUrl);
    void setNetworkProxyHost(const QString& host);
    void setNetworkProxyPort(QString host);
    void setNetworkProxyUseCredentials(bool useCredentials);
    void setNetworkProxyType(NetworkProxyType type);
    void setNetworkProxyUsername(const QString& username);
    void setNetworkProxyPassword(const QString& password);
    void setProjectTmplatesAutoUpdate(bool automatic);

    QHash<QString, std::function<QString ()> > filterTextMap;
};

#endif // APPCONFIG_H
