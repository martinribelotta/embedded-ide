#include "codeeditor.h"
#include "mapviewer.h"

#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsProxyWidget>
#include <QProgressBar>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <QtDebug>

MapViewer::MapViewer(QWidget *parent) : QGraphicsView(parent)
{
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    setScene(new QGraphicsScene(this));
}

static const QLatin1String MEMORY_MAP_HEADER("Memory Configuration");
static const QLatin1String REGEX_MEM(R"(^(?P<name>\S+)\s+)"
                                     R"((?P<origin>0x[0-9a-fA-F]+)\s+)"
                                     R"((?P<length>0x[0-9a-fA-F]+)\s+)"
                                     R"((?P<attr>[xrw]+)$)");
static const QLatin1String REGEX_SEC(R"(^(?!\.debug|\.comment|.*?attributes)(?P<name>\.\S+)\s+)"
                                     R"((?P<vma>0x[a-f0-9]+)\s+)"
                                     R"((?!0x0)(?P<size>0x[a-f0-9]+))"
                                     R"((?:\s+load address (?P<lma>0x[a-z0-9]+))?)");

struct MemoryChunk {
    QString name;
    uint32_t addr;
    uint32_t size;

    uint32_t hi_addr() const {
        return addr + size;
    }
};

inline QDebug operator<<(QDebug dbg, const MemoryChunk& chunk) {
    dbg << chunk.name
        << ":{ addr:" << chunk.addr
        << "size:" << chunk.size
        << " }";
    return dbg;
}

struct MemoryRegion {
    QString name;
    uint32_t base;
    uint32_t size;
    QString attr;
    uint32_t used;
    uint32_t pad;

    uint32_t upper() const {
        return base + size;
    }

    bool inRegion(const MemoryChunk& chunk) const {
        return (chunk.addr >= base) && (chunk.hi_addr() <= upper());
    }
};

inline QDebug operator<<(QDebug dbg, const MemoryRegion& m) {
    dbg << m.name
        << ":{ base:" << m.base
        << "size:" << m.size
        << "attr:" << m.attr
        << " }";
    return dbg;
}

class MemoryBar: public QGraphicsProxyWidget {
public:
    MemoryBar(const MemoryRegion& m, QGraphicsItem *parent = 0l);
    virtual ~MemoryBar();
};

MemoryBar::MemoryBar(const MemoryRegion& m, QGraphicsItem *parent) :
    QGraphicsProxyWidget(parent)
{
    auto bar = new QProgressBar();
    setWidget(bar);
    bar->setFormat(tr("%1 using %v bytes of %m bytes (%p%)").arg(m.name));
    bar->setTextVisible(true);
    bar->setMinimum(0);
    bar->setMaximum(static_cast<int>(m.size));
    bar->setValue(static_cast<int>(m.used));
    bar->resize(QFontMetrics(bar->font()).width(bar->text()) + 100, bar->height());
    auto effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(8);
    effect->setColor(Qt::black);
    effect->setOffset(0, 0);
    setGraphicsEffect(effect);
}

MemoryBar::~MemoryBar() {}

bool MapViewer::load(const QString &path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly)) {
        scene()->clear();
        QString text(f.readAll());
        int memHeaderIdx = text.indexOf(MEMORY_MAP_HEADER);
        if (memHeaderIdx > 0) {
            QList<MemoryRegion> memoryRegions;
            auto re_mem = QRegularExpression(REGEX_MEM, QRegularExpression::CaseInsensitiveOption |
                                                        QRegularExpression::MultilineOption);
            auto mem_matchs = re_mem.globalMatch(text, memHeaderIdx);
            while (mem_matchs.hasNext()){
                auto m = mem_matchs.next();
                auto memName = m.captured("name");
                auto memOrg = m.captured("origin").remove(0, 2).toUInt(nullptr, 16);
                auto memLen = m.captured("length").remove(0, 2).toUInt(nullptr, 16);
                auto memAttr = m.captured("attr");
                memoryRegions.append(MemoryRegion({memName, memOrg, memLen, memAttr, 0, 0 }));
            }

            auto re_sec = QRegularExpression(REGEX_SEC, QRegularExpression::CaseInsensitiveOption |
                                                        QRegularExpression::MultilineOption);
            auto sec_matchs = re_sec.globalMatch(text, memHeaderIdx);
            while (sec_matchs.hasNext()) {
                auto m = sec_matchs.next();
                auto secName = m.captured("name");
                auto secVma = m.captured("vma").remove(0, 2).toUInt(nullptr, 16);
                auto secSize = m.captured("size").remove(0, 2).toUInt(nullptr, 16);
                auto secLma = m.lastCapturedIndex() == 4? m.captured("lma").remove(0, 2).toUInt(nullptr, 16) : secVma;
                auto chunk = MemoryChunk({secName, secLma, secSize});
                qDebug() << chunk;
                for(int i=0; i<memoryRegions.count(); i++) {
                    MemoryRegion &r = memoryRegions[i];
                    if (r.inRegion(chunk))
                        r.used += chunk.size;
                }
            }
            int maxWidth = 0;
            foreach(auto r, memoryRegions) {
                auto item_bar = new MemoryBar(r);
                scene()->addItem(item_bar);
                maxWidth = qMax(maxWidth, item_bar->widget()->width());
            }
            int dy = 0;
            foreach(QGraphicsItem *item, scene()->items()) {
                auto bar = qobject_cast<QProgressBar*>(qgraphicsitem_cast<QGraphicsProxyWidget*>(item)->widget());
                bar->resize(maxWidth, bar->height());
                item->moveBy(0, dy);
                dy += item->boundingRect().height() + 10;
            }
#if 0
            auto editor = new CodeEditor();
            editor->load(path);
            auto editorWidget = scene()->addWidget(editor);
            editorWidget->setPos(0, dy + 10);
            editorWidget->resize(maxWidth, editorWidget->size().height());
#endif
            return true;
        }
    }
    return false;
}
