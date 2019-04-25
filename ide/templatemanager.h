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
#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

#include <QUrl>
#include <QWidget>

namespace Ui {
class TemplateManager;
}

class QListWidgetItem;
class TemplateItemWidget;

class TemplateManager : public QWidget
{
    Q_OBJECT

public:
    explicit TemplateManager(QWidget *parent = nullptr);
    virtual ~TemplateManager();

    QUrl repositoryUrl() const;

    QList<TemplateItemWidget*> itemWidgets() const;

signals:
    void errorMessage(const QString& text);
    void message(const QString& text);
    void haveMetadata();

public slots:
    void setRepositoryUrl(const QUrl& url);
    void startUpdate();

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
