#include "filesystemmanager.h"

#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QMimeDatabase>
#include <QTreeView>
#include <QWidget>

class ProjectIconProvider: public QFileIconProvider
{
public:
    QIcon icon(const QFileInfo &info) const
    {
        QMimeDatabase db;
        QMimeType t = db.mimeTypeForFile(info);
        if (t.isValid()) {
            QString resName = QString(":/images/mimetypes/%1.svg").arg(t.iconName());
            if (QFile(resName).exists())
                return QIcon(resName);
            resName = QString(":/images/mimetypes/%1.svg").arg(t.genericIconName());
            if (QFile(resName).exists())
                return QIcon(resName);
        }
        return QFileIconProvider::icon(info);
    }
};

FilesystemManager::FilesystemManager(QTreeView *v, QObject *parent) : QObject(parent), view(v)
{
    connect(view, &QTreeView::activated, [this](const QModelIndex& idx) {
        auto model = qobject_cast<QFileSystemModel*>(view->model());
        if (model) {
            auto path = model->filePath(idx);
            emit requestFileOpen(path);
        }
    });
}

void FilesystemManager::openPath(const QString &path)
{
    if (view) {
        auto model = qobject_cast<QFileSystemModel*>(view->model());
        if (!model) {
            if (view->model())
                view->model()->deleteLater();
            view->setModel(model = new QFileSystemModel(view));
        }
        model->setIconProvider(new ProjectIconProvider);
        view->setRootIndex(model->setRootPath(path));
        for(int i=1; i<model->columnCount(); i++)
            view->hideColumn(i);
        view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        view->header()->hide();
    }

}

void FilesystemManager::closePath()
{
    if (view->model())
        view->model()->deleteLater();
    view->setModel(nullptr);
}
