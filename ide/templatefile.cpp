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
#include <zlib.h>

#include <QRegularExpressionMatch>

static const QStringList DIFF_EXTENTIONS = {
    "template",
    "diff",
    "patch"
};

static const QStringList METADATA_FILENAMES = {
    "MANIFEST.md",
    "README.md",
    "MANIFEST.html",
    "README.html",
    "MANIFEST.htm",
    "README.htm",
    "MANIFEST.txt",
    "README.txt",
    "MANIFEST",
    "README"
};

const QString TemplateFile::TEMPLATE_FILEDIALOG_FILTER =
    QObject::tr("All knowed template formats (*.template *.tar.gz);;"
                "Templates (*.template);;"
                "Compressed tar with gzip (*.tar.gz);;"
                "All files (*)");

TemplateFile::TemplateFile(const QFileInfo& path)
    : info(path)
{
}

TemplateFile::Type TemplateFile::type() const
{
    if (info.completeSuffix().toLower() == "tar.gz")
        return Type::TarGzFile;
    if (DIFF_EXTENTIONS.contains(info.suffix().toLower()))
        return Type::DiffFile;
    return Type::Unknown;
}

template<typename T1, typename T2>
static T1 roundupTo(T1 v, T2 n)
{
    v = (((v) + (n - 1)) & ~(n - 1));
    return v;
}

DiffFile::DiffFile(const QString &path)
{
    constexpr auto RE_E = R"(\$\{\{(?P<name>[a-zA-Z_0-9]+)(?:\s+(?P<type>string|items)\:(?P<params>.*?))?\}\})";
    name = path;
    diffText = AppConfig::readEntireTextFile(name);
    auto m = QRegularExpression(R"((^[\s\S]*?)^diff )", QRegularExpression::MultilineOption).match(diffText);
    int startOfDiff = m.hasMatch()? m.capturedEnd(1) : 0;
    if (startOfDiff > 0)
        manifest = diffText.mid(0, startOfDiff - 1);
    QRegularExpression re(RE_E, QRegularExpression::MultilineOption);
    for (auto it = re.globalMatch(diffText, startOfDiff); it.hasNext(); ) {
        auto m = it.next();
        QString name = m.captured("name");
        QString type = m.captured("type");
        QString params = m.captured("params");
        if (!type.isEmpty() && !params.isEmpty())
            parameters.append({ name, type, params });
    }
}

TarGzFile::TarGzFile(const QString &path)
{
    gzFile f = gzopen(path.toLocal8Bit().constData(), "rb");
    if (f) {
        posix_header h{};
        do {
            auto len = gzread(f, reinterpret_cast<char*>(&h), sizeof(h));
            if (len == sizeof(h)) {
                if (QLatin1Literal(h.magic, TMAGLEN) == "ustar ") {
                    QFileInfo finfo(h.name);
                    bool ok = false;
                    constexpr auto OCT_RADIX = 8;
                    auto size = QString(h.size).toLong(&ok, OCT_RADIX);
                    if (!ok)
                        break;
                    auto pos = gztell(f);
                    auto name = finfo.fileName();
                    if (METADATA_FILENAMES.contains(name)) {
                        QByteArray buffer;
                        buffer.resize(static_cast<int>(size));
                        auto *ptr = buffer.data();
                        auto readed = gzread(f, ptr, static_cast<unsigned int>(size));
                        buffer.resize(readed);
                        metadata = QString::fromLocal8Bit(buffer);
                        metadataSuffix = finfo.suffix();
                        break;
                    }
                    constexpr auto CHUNCK_SIZE = 512;
                    size = roundupTo(size, CHUNCK_SIZE);
                    gzseek(f, pos + size, SEEK_SET);
                } else {
                    break;
                }
            } else {
                break;
            }
        } while(!gzeof(f));
    }
}
