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
    void adjustEnv();

public:
    static AppConfig &instance();
    static QString replaceWithEnv(const QString& str);
    static QByteArray readEntireTextFile(const QString& path);
    static QIODevice *writeEntireTextFile(const QString& text, const QString& path);
    static const QString& ensureExist(const QString& d);
    static QStringList langList();
    static QStringList langPaths();
    static QString resourceImage(const QString& path, const QString& ext="svg");
    static QString resourceImage(const QStringList& pathPart, const QString& ext="svg");
    static void fixIconTheme(QWidget *w);

    enum class NetworkProxyType { None, System, Custom };
    Q_ENUM(NetworkProxyType)

    QString workspacePath() const;
    QString projectsPath() const;
    QString templatesPath() const;
    QString localConfigFilePath() const;

    QList<QPair<QString, QString> > externalTools() const;
    QFileInfoList recentProjects() const;

    QStringList additionalPaths(bool raw = false) const;
    QStringList additionalRawPaths() const { return additionalPaths(true); }

    QString templatesUrl() const;

    QString editorStyle() const;
    QFont editorFont() const;
    bool editorSaveOnAction() const;
    bool editorTabsToSpaces() const;
    int editorTabWidth() const;
    QString editorFormatterStyle() const;
    QString editorFormatterExtra() const;
    bool editorDetectIdent() const;

    QFont loggerFont() const;

    QString networkProxyHost() const;
    QString networkProxyPort() const;
    bool networkProxyUseCredentials() const;
    NetworkProxyType networkProxyType() const;
    QString networkProxyUsername() const;
    QString networkProxyPassword() const;

    bool projectTemplatesAutoUpdate() const;

    bool useDevelopMode() const;
    bool useDarkStyle() const;

    QString language() const;

    QByteArray fileHash(const QString& filename);

signals:
    void configChanged(AppConfig*);

public slots:
    void load();
    void save();

    void setWorkspacePath(const QString& path);

    void setExternalTools(const QList<QPair<QString, QString> > &tools);
    void appendToRecentProjects(const QString& path);

    void setAdditionalPaths(const QStringList& paths);

    void setTemplatesUrl(const QString& url);

    void setEditorStyle(const QString& name);
    void setEditorFont(const QFont& f);
    void setEditorSaveOnAction(bool enable);
    void setEditorTabsToSpaces(bool enable);
    void setEditorTabWidth(int n);
    void setEditorFormatterStyle(const QString& name);
    void setEditorFormatterExtra(const QString& text);
    void setEditorDetectIdent(bool enable);

    void setLoggerFont(const QFont& f);

    void setNetworkProxyHost(const QString& name);
    void setNetworkProxyPort(const QString& port);
    void setNetworkProxyUseCredentials(bool use);
    void setNetworkProxyType(NetworkProxyType type);
    void setNetworkProxyUsername(const QString& user);
    void setNetworkProxyPassword(const QString& pass);

    void setProjectTemplatesAutoUpdate(bool en);

    void setUseDevelopMode(bool use);
    void setUseDarkStyle(bool use);
    void setLanguage(const QString& lang);

    void addHash(const QString& filename, const QByteArray& hash);
    void purgeHash();
};

#endif // APPCONFIG_H
