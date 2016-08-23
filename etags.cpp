#include "etags.h"

#include <QFile>
#include <QtDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QtConcurrent>

static bool findNextEntry(QFile *f)
{
    bool founded = false;
    while(!f->atEnd() && !founded) {
        char c;
        qint64 r = f->read(&c, 1);
        if (r == -1)
            break;
        if (r == 1 && c == 0x0C)
            founded = (QChar(f->read(1).at(0)) == '\n');
        if (r == 0)
            break;
    }
    return founded;
}

static QString processFileName(const QString& line, int *len)
{
    QStringList parts = line.split(',');
    if (parts.count() == 2) {
        QString file = parts.at(0);
        bool ok;
        *len = parts.at(1).toInt(&ok, 10);
        if (ok) {
            return file;
        }
    }
    *len = -1;
    return QString();
}

static void parseDefs(const QString& in, const QString& file, ETags::TagMap *map)
{
    QRegularExpression re("([^\\x7f]+)\\x7f([^\\x01]+)\\x01(\\d+),(\\d+)\\n");
    QRegularExpressionMatchIterator it = re.globalMatch(in);
    while(it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        ETags::Tag ctx;
        QString tag = m.captured(2);
        ctx.decl = m.captured(1);
        ctx.file = file;
        ctx.line = m.captured(3).toInt();
        ctx.offset = m.captured(4).toInt();
        map->insert(tag, ctx);
    }
}

bool ETags::parse(const QString &path)
{
    m_path = path;
    QFile in(m_path);
    map.clear();
    if (in.open(QFile::ReadOnly)) {
        while (findNextEntry(&in)) {
            QByteArray line = in.readLine();
            int len = 0;
            QString file = processFileName(line, &len);
            if (!file.isEmpty() && len != -1) {
                QString defs = in.read(len);
                parseDefs(defs, file, &map);
            }
        }
        return true;
    } else
        return false;
}

void ETags::etagsStart(const QString& workDir, std::function<void(ETags& tag)> callback)
{
    QtConcurrent::run([workDir, callback] () {
        QProcess ctags;
        ctags.setWorkingDirectory(workDir);
        ctags.start("ctags", QStringList() << "-R" << "-e");
        if (ctags.waitForStarted()) {
            if (ctags.waitForFinished()) {
                ETags tagFile;
                if (tagFile.parse(workDir + QDir::separator() + "TAGS")) {
                    callback(tagFile);
                } else
                    qDebug() << "error parsing TAGS file";
            } else
                qDebug() << "ctags termination timeout" << ctags.errorString();
        } else
            qDebug() << "Error on start ctags " << ctags.errorString();
    });
}
