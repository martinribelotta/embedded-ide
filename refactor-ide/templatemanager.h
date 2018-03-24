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
    explicit TemplateManager(QWidget *parent = 0);
    virtual ~TemplateManager();

    QUrl repositoryUrl() const;

signals:
    void errorMessage(const QString& text);
    void message(const QString& text);

public slots:
    void setRepositoryUrl(const QUrl& url);

protected:
    void showEvent(QShowEvent *event);

private:
    Ui::TemplateManager *ui;
    QHash<QString, QListWidgetItem*> itemList;

    void updateLocalTemplates();
};

#endif // TEMPLATEMANAGER_H
