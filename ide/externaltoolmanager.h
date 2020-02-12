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
#ifndef EXTERNALTOOLMANAGER_H
#define EXTERNALTOOLMANAGER_H

#include <QDialog>

#include <memory>

namespace Ui {
class ExternalToolManager;
}

class QMenu;

class ProcessManager;
class ProjectManager;

class ExternalToolManager : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(ExternalToolManager)

public:
    explicit ExternalToolManager(QWidget *parent = nullptr);
    virtual ~ExternalToolManager();

    static QMenu *makeMenu(QWidget *parent, ProcessManager *pman, ProjectManager *proj);

private:
    std::unique_ptr<Ui::ExternalToolManager> ui;
};

#endif // EXTERNALTOOLMANAGER_H
