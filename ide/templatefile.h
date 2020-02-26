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
#ifndef TEMPLATEFILE_H
#define TEMPLATEFILE_H

#include <QString>
#include <QFileInfo>
#include <QHash>

struct DiffParameter {
    QString name;
    QString type;
    QString params;
};

struct DiffFile {
    QString name;
    QString manifest;
    QString diffText;
    QString headerText;
    QList<DiffParameter> parameters;

    DiffFile(const QString& path);
};

struct TarGzFile {
    QString metadataSuffix;
    QString metadata;

    TarGzFile(const QString& path);
};

class TemplateFile
{
public:
    enum class Type {
        Unknown,
        DiffFile,
        TarGzFile,
    };

    static const QString TEMPLATE_FILEDIALOG_FILTER;

    TemplateFile(const QFileInfo& path);

    Type type() const;

private:
    QFileInfo info;
};

#endif // TEMPLATEFILE_H
