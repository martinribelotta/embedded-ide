#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "configdialog.h"
#include "dialogconfigworkspace.h"

class AppConfig
{
    // WARNING(denisacostaq@gmail.com):
    // These classes can access to the private constructor, but do not create
    // an instance through this, use mutableInstance instead.
    friend ConfigDialog;
    friend DialogConfigWorkspace;
  public:
    const QStringList& buildAdditionalPaths() const;
    const QString& editorStyle() const;
    int editorFontSize() const;
    const QString& editorFontStyle() const;
    bool editorSaveOnAction() const;
    bool editorTabsToSpaces() const;
    int editorTabWidth() const;
    const QString& builDefaultProjectPath() const;
    const QString& builTemplatePath() const;
    const QString& builTemplateUrl() const;
    QString defaultApplicationResources();
    QString defaultProjectPath();
    QString defaultTemplatePath();
    QString defaultTemplateUrl();

    void adjustPath();

    static AppConfig& mutableInstance();
  private:
    AppConfig();
    void load();
    void save();

    void setBuildAdditionalPaths(const QStringList& buildAdditionalPaths) const;
    void setEditorStyle(const QString& editorStyle);
    void setEditorFontSize(int editorFontSize);
    void setEditorFontStyle(const QString& editorFontStyle);
    void setEditorSaveOnAction(bool editorSaveOnAction);
    void setEditorTabsToSpaces(bool editorTabsToSpaces);
    void setEditorTabWidth(int editorTabWidth);
    void setBuilDefaultProjectPath(const QString& builDefaultProjectPath);
    void setBuilTemplatePath(const QString& builTemplatePath);
    void setBuilTemplateUrl(const QString& builTemplateUrl);
};

#endif // APPCONFIG_H
