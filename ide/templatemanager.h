#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

#include <QUrl>
#include <QWidget>

namespace Ui {
class TemplateManager;
}

class QListWidgetItem;

class TemplateManager : public QWidget
{
    Q_OBJECT

public:
    explicit TemplateManager(QWidget *parent = nullptr);
    virtual ~TemplateManager();

    QUrl repositoryUrl() const;

signals:
    void errorMessage(const QString& text);
    void message(const QString& text);

public slots:
    void setRepositoryUrl(const QUrl& url);

private slots:
    void msgLog(const QString& text, const QColor& color);
    void logError(const QString& text);
    void logMsg(const QString& text);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::TemplateManager *ui;
    QHash<QString, QListWidgetItem*> itemList;

    void updateLocalTemplates();
};

#endif // TEMPLATEMANAGER_H
