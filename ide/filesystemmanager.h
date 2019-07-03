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
#ifndef FILESYSTEMMANAGER_H
#define FILESYSTEMMANAGER_H

#include <QFileInfo>
#include <QObject>

class QTreeView;

class FileSystemManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileSystemManager)

public:
    explicit FileSystemManager(QTreeView *v, QObject *parent = nullptr);
    virtual ~FileSystemManager();

    static QIcon iconForFile(const QFileInfo &info);

    static QString mimeIconPath(const QString& mimeName);

signals:
    void requestFileOpen(const QString& path);

public slots:
    void openPath(const QString& path);
    void closePath();

private slots:
    void customContextMenu(const QPoint& pos);

    void menuNewFile();
    void menuNewDirectory();
    void menuNewSymlink();
    void menuItemExecute();
    void menuItemOpenExternal();
    void menuItemRename();
    void menuItemDelete();

private:
    QTreeView *view;
};

#endif // FILESYSTEMMANAGER_H
