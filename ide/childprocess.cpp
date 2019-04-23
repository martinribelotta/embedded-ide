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
#include "childprocess.h"

QProcess *ChildProcess::safeStop(QProcess *p)
{
    p->blockSignals(true);
    if (p->state() == QProcess::Running) {
        p->terminate();
        p->waitForFinished(100);
        if (p->state() == QProcess::Running) {
            p->kill();
            p->waitForFinished(100);
        }
    }
    p->blockSignals(false);
    return p;
}

ChildProcess::~ChildProcess() {
    safeStop(this);
}
