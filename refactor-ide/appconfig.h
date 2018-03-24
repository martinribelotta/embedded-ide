#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <QDir>

class AppConfig : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AppConfig)

private:
    class Priv_t;
    Priv_t *priv;

    explicit AppConfig();
    virtual ~AppConfig();

public:
    static AppConfig &instance();
    static void adjustEnv();
    static QString replaceWithEnv(const QString& str);
    static QByteArray readEntireTextFile(const QString& path);
    static const QString& ensureExist(const QString& d);

    enum class NetworkProxyType { None, System, Custom };
    Q_ENUM(NetworkProxyType)

    QString workspacePath() const;
    QString projectsPath() const;
    QString templatesPath() const;
    QString localConfigFilePath() const;

    QHash<QString, QString> externalTools() const;
    QFileInfoList recentProjects() const;

    QStringList additionalPaths() const;

    QString templatesUrl() const;

    QString editorStyle() const;
    QFont editorFont() const;
    bool editorSaveOnAction() const;
    bool editorTabsToSpaces() const;
    int editorTabWidth() const;
    QString editorFormatterStyle() const;

    QFont loggerFont() const;

    QString networkProxyHost() const;
    QString networkProxyPort() const;
    bool networkProxyUseCredentials() const;
    NetworkProxyType networkProxyType() const;
    QString networkProxyUsername() const;
    QString networkProxyPassword() const;

    bool projectTemplatesAutoUpdate() const;

    bool useDevelopMode() const;

    QByteArray fileHash(const QString& filename);

signals:
    void configChanged(AppConfig*);

public slots:
    void load();
    void save();

    void setWorkspacePath(const QString& path);

    void setExternalTools(const QHash<QString, QString> &tools);
    void appendToRecentProjects(const QString& path);

    void setAdditionalPaths(const QStringList& paths);

    void setTemplatesUrl(const QString& url);

    void setEditorStyle(const QString& name);
    void setEditorFont(const QFont& f);
    void setEditorSaveOnAction(bool enable);
    void setEditorTabsToSpaces(bool enable);
    void setEditorTabWidth(int n);
    void setEditorFormatterStyle(const QString& name);

    void setLoggerFont(const QFont& f);

    void setNetworkProxyHost(const QString& name);
    void setNetworkProxyPort(const QString& port);
    void setNetworkProxyUseCredentials(bool use);
    void setNetworkProxyType(NetworkProxyType type);
    void setNetworkProxyUsername(const QString& user);
    void setNetworkProxyPassword(const QString& pass);

    void setProjectTemplatesAutoUpdate(bool en);

    void setUseDevelopMode(bool use);

    void addHash(const QString& filename, const QByteArray& hash);
    void purgeHash();
};

#endif // APPCONFIG_H
