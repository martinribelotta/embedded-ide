#include "mapfileviewer.h"

#include <QHeaderView>
#include <mapviewmodel.h>

MapFileViewer::MapFileViewer(QWidget *parent) : QTreeView(parent)
{
    setAlternatingRowColors(true);
    setEditTriggers(QTreeView::NoEditTriggers);
    setItemDelegateForColumn(MapViewModel::PERCENT_COLUMN, new BarItemDelegate(this));
    auto adjust = [this](const QModelIndex&) { header()->resizeSections(QHeaderView::ResizeToContents); };
    QObject::connect(this, &QTreeView::expanded, adjust);
    QObject::connect(this, &QTreeView::collapsed, adjust);
}

MapFileViewer::~MapFileViewer()
= default;

bool MapFileViewer::load(const QString &path)
{
    auto model = new MapViewModel(this);
    setModel(model);
    if (!model->load(path))
        return false;
    header()->resizeSections(QHeaderView::ResizeToContents);
    setPath(path);
    return true;
}

class MAPEditorCreator: public IDocumentEditorCreator
{
public:
    ~MAPEditorCreator() override;
    bool canHandleExtentions(const QStringList &suffixes) const override {
        for(const QString& suffix: suffixes)
            if (suffix.compare("map", Qt::CaseInsensitive) == 0)
                return true;
        return false;
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new MapFileViewer(parent);
    }
};

IDocumentEditorCreator *MapFileViewer::creator()
{
    return IDocumentEditorCreator::staticCreator<MAPEditorCreator>();
}

MAPEditorCreator::~MAPEditorCreator()
= default;
