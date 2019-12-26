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
#include "templatefile.h"
#include "tar.h"
#include "appconfig.h"

#include <QRegularExpressionMatch>

static const QStringList DIFF_EXTENTIONS = {
    "template",
    "diff",
    "patch"
};

static const QStringList METADATA_FILENAMES = {
    "MANIFEST.txt",
    "MANIFEST.md",
    "MANIFEST.html",
    "MANIFEST.htm",
    "MANIFEST",
    "README.txt",
    "README.md",
    "README.html",
    "README.htm",
    "README"
};

TemplateFile::TemplateFile(const QFileInfo& path)
    : info(path), infometa(extractMeta(path.absoluteFilePath()))
{
}

TemplateFile::Type TemplateFile::type() const
{
    if (!infometa.isEmpty())
        return Type::TarGzWithMetaFile;
    if (info.completeSuffix().toLower() == "tar.gz")
        return Type::TarGzFile;
    if (DIFF_EXTENTIONS.contains(info.suffix().toLower()))
        return Type::DiffFile;
    return Type::Unknown;
}

template<typename T1, typename T2>
static auto roundupTo(T1 v, T2 n)
{
    v = (((v) + (n - 1)) & ~(n - 1));
    return v;
}

TemplateFile::Metadata TemplateFile::extractMeta(const QString &path)
{
    Metadata meta;
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
        posix_header h{};
        do {
            auto len = f.read(reinterpret_cast<char*>(&h), sizeof(h));
            if (len == sizeof(h)) {
                if (QLatin1Literal(h.magic, TMAGLEN) == "ustar ") {
                    QFileInfo finfo(h.name);
                    bool ok = false;
                    constexpr auto octRadix = 8;
                    auto size = QString(h.size).toLong(&ok, octRadix);
                    if (!ok)
                        break;
                    auto pos = f.pos();
                    if (METADATA_FILENAMES.contains(finfo.filePath())) {
                        auto data = f.read(size);
                        meta.insert(finfo.filePath(), data);
                    }
                    constexpr auto CHUNCK_SIZE = 512;
                    size = roundupTo(size, CHUNCK_SIZE);
                    f.seek(pos + size);
                } else {
                    break;
                }
            } else {
                break;
            }
        } while(!f.atEnd());
    }
    return meta;
}

DiffFile::DiffFile(const QString &path)
{
    name = path;
    diffText = AppConfig::readEntireTextFile(name);
    QRegularExpressionMatch m = QRegularExpression(R"((^[\s\S]*?)^diff )", QRegularExpression::MultilineOption)
                                    .match(diffText);
    int startOfDiff = m.hasMatch()? m.capturedEnd(1) : 0;
    if (startOfDiff > 0)
        manifest = diffText.mid(0, startOfDiff - 1);
    QRegularExpression re(R"(\$\{\{(?P<name>[a-zA-Z_0-9]+)(?:\s+(?P<type>string|items)\:(?P<params>.*?))?\}\})",
                          QRegularExpression::MultilineOption);
    auto it = re.globalMatch(diffText, startOfDiff);
    while (it.hasNext()) {
        auto m = it.next();
        QString name = m.captured("name");
        QString type = m.captured("type");
        QString params = m.captured("params");
        if (!type.isEmpty() && !params.isEmpty())
            parameters.append({ name, type, params });
    }
}
