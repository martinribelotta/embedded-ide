#include "mapviewer.h"

#include <QFile>
#include <QGraphicsRectItem>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <QtDebug>

MapViewer::MapViewer(QWidget *parent) : QGraphicsView(parent)
{
    setScene(new QGraphicsScene(this));
    scene()->addRect(QRect(0, 0, 100, 100), QPen(Qt::red), QBrush(Qt::blue));
}

static const QLatin1String MEMORY_MAP_HEADER("Memory Configuration");
static const QLatin1String regex_mem(R"(^(?P<name>\S+)\s+(?P<origin>0x[0-9a-fA-F]+)\s+(?P<length>0x[0-9a-fA-F]+)\s+(?P<attr>[xrw]+)$)");
static const QLatin1String regex_sec(R"(^(?!\.debug|\.comment|.*?attributes)(?P<name>\.\S+)\s+(?P<vma>0x[a-f0-9]+)\s+(?!0x0)(?P<size>0x[a-f0-9]+)(?:\s+load address (?P<lma>0x[a-z0-9]+))?)");

struct MemoryChunk {
    QString name;
    uint32_t addr;
    uint32_t size;

    uint32_t hi_addr() const {
        return addr + size;
    }
};

struct MemoryRegion {
    QString name;
    uint32_t base;
    uint32_t size;
    QString attr;
    uint32_t used;

    uint32_t upper() const {
        return base + size;
    }

    bool inRegion(const MemoryChunk& chunk) const {
        return (chunk.addr >= base) && (chunk.hi_addr() <= upper());
    }
};

bool MapViewer::load(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
        QString text(f.readAll());
        int memHeaderIdx = text.indexOf(MEMORY_MAP_HEADER);
        if (memHeaderIdx > 0) {
            QList<MemoryRegion> memoryRegions;
            auto re_mem = QRegularExpression(regex_mem, QRegularExpression::CaseInsensitiveOption |
                                                        QRegularExpression::MultilineOption);
            auto mem_matchs = re_mem.globalMatch(text, memHeaderIdx);
            qDebug() << "memory areas" << mem_matchs.hasNext();
            while (mem_matchs.hasNext()){
                auto m = mem_matchs.next();
                auto memName = m.captured("name");
                auto memOrg = m.captured("origin").remove(0, 2).toUInt(nullptr, 16);
                auto memLen = m.captured("length").remove(0, 2).toUInt(nullptr, 16);
                auto memAttr = m.captured("attr");
                qDebug() << "   " << m.captured(0);
                qDebug() << "   " << memName << memOrg << memLen << memAttr;
                memoryRegions.append(MemoryRegion({memName, memOrg, memLen, memAttr, 0}));
            }

            auto re_sec = QRegularExpression(regex_sec, QRegularExpression::CaseInsensitiveOption |
                                                        QRegularExpression::MultilineOption);
            auto sec_matchs = re_sec.globalMatch(text, memHeaderIdx);
            qDebug() << "valid sections" << sec_matchs.hasNext();
            while (sec_matchs.hasNext()) {
                auto m = sec_matchs.next();
                auto secName = m.captured("name");
                auto secVma = m.captured("vma").remove(0, 2).toUInt(nullptr, 16);
                auto secSize = m.captured("size").remove(0, 2).toUInt(nullptr, 16);
                auto secLma = m.lastCapturedIndex() == 4? m.captured("lma").remove(0, 2).toUInt(nullptr, 16) : secVma;
                qDebug() << "   " << m.captured(0);
                qDebug() << "   " << secName << secLma << secSize << secVma;
                auto chunk = MemoryChunk({secName, secLma, secSize});
                for(int i=0; i<memoryRegions.count(); i++) {
                    MemoryRegion &r = memoryRegions[i];
                    if (r.inRegion(chunk))
                        r.used += chunk.size;
                }
            }
            foreach(auto r, memoryRegions) {
                qDebug() << r.name << "used" << r.used << "of" << r.size;
            }

            return true;
        }
    }
    return false;
}
