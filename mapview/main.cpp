#include "mapviewmodel.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QHeaderView>
#include <QTreeView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser opt;
    opt.addPositionalArgument("filename", "Map file");
    opt.addHelpOption();
    opt.addVersionOption();
    opt.process(app);

    if (opt.positionalArguments().isEmpty())
        opt.showHelp(-1);

    QTreeView view;
    auto model = new MapViewModel(&view);
    view.setAlternatingRowColors(true);
    view.setEditTriggers(QTreeView::NoEditTriggers);
    QObject::connect(&view, &QTreeView::expanded, [&view](const QModelIndex&) {
        view.header()->resizeSections(QHeaderView::ResizeToContents);
    });
    view.setModel(model);
    model->load(opt.positionalArguments().first());
    view.header()->resizeSections(QHeaderView::ResizeToContents);
    view.setItemDelegateForColumn(MapViewModel::PERCENT_COLUMN, new BarItemDelegate(&view));
    view.show();
    return app.exec();
}
