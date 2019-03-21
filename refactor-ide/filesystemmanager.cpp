#include "filesystemmanager.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QProcess>
#include <QTreeView>
#include <QUrl>
#include <QWidget>

#include <QtDebug>

class ProjectIconProvider: public QFileIconProvider
{
public:
    ~ProjectIconProvider() override;
    QIcon icon(const QFileInfo &info) const override
    {
        QMimeDatabase db;
        auto t = db.mimeTypeForFile(info);
        if (t.isValid()) {
            auto resName = QString(":/images/mimetypes/%1.svg").arg(t.iconName());
            if (QFile(resName).exists())
                return QIcon(resName);
            resName = QString(":/images/mimetypes/%1.svg").arg(t.genericIconName());
            if (QFile(resName).exists())
                return QIcon(resName);
        }
        return QFileIconProvider::icon(info);
    }
};

class FileSystemModel: public QFileSystemModel
{
public:
    ~FileSystemModel() override;
    FileSystemModel(QObject *parent = nullptr) : QFileSystemModel(parent)
    {
        setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden | QDir::System);
        setNameFilterDisables(false);
        setNameFilters({ "*" });
        setIconProvider(new ProjectIconProvider);
        setReadOnly(false);
    }
};

FileSystemManager::FileSystemManager(QTreeView *v, QObject *parent) : QObject(parent), view(v)
{
    connect(view, &QTreeView::activated, [this](const QModelIndex& idx) {
        auto model = qobject_cast<QFileSystemModel*>(view->model());
        if (model) {
            auto path = model->filePath(idx);
            emit requestFileOpen(path);
        }
    });
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view, &QTreeView::customContextMenuRequested, this, &FileSystemManager::customContextMenu);
}

FileSystemManager::~FileSystemManager()
= default;

QIcon FileSystemManager::iconForFile(const QFileInfo &info)
{
    return ProjectIconProvider().icon(info);
}

void FileSystemManager::openPath(const QString &path)
{
    if (view) {
        auto model = qobject_cast<QFileSystemModel*>(view->model());
        if (!model) {
            if (view->model())
                view->model()->deleteLater();
            view->setModel(model = new FileSystemModel(view));
        }
        view->setRootIndex(model->setRootPath(path));
        for(int i = 1; i < model->columnCount(); i++)
            view->hideColumn(i);
        view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        view->header()->hide();
    }

}

void FileSystemManager::closePath()
{
    if (view->model())
        view->model()->deleteLater();
    view->setModel(nullptr);
}

static bool isExec(const QFileInfo& f)
{
    return f.isExecutable() && !f.isDir();
}

void FileSystemManager::customContextMenu(const QPoint &pos)
{
    auto model = qobject_cast<QFileSystemModel*>(view->model());
    if (!model)
        return;

    auto index = view->currentIndex();
    if (!index.isValid())
        index = view->rootIndex();

    auto noSelection = view->selectionModel()->selectedRows(0).isEmpty();
    auto info = model->fileInfo(index);
    auto m = new QMenu(view);
#define _(icon, text, slot) \
    m->addAction(QIcon(":/images/actions/" icon ".svg"), text, this, slot)

    _("document-new", tr("New File"), &FileSystemManager::menuNewFile);
    _("folder-new", tr("New Directory"), &FileSystemManager::menuNewDirectory);
#ifdef Q_OS_UNIX
    _("insert-link-symbolic", tr("New Symlink"), &FileSystemManager::menuNewSymlink);
#endif
    _("edit-find", tr("Properties"), &FileSystemManager::menuItemProperties);
    _("run-build-file", tr("Execute"), &FileSystemManager::menuItemExecute)->setEnabled(isExec(info));
    _("window-new", tr("Open External"), &FileSystemManager::menuItemOpenExternal);
    m->addSeparator();
    _("debug-execute-from-cursor", tr("Rename"), &FileSystemManager::menuItemRename)->setDisabled(noSelection);
    _("document-close", tr("Delete"), &FileSystemManager::menuItemDelete)->setDisabled(noSelection);
#undef _
    m->exec(view->mapToGlobal(pos));
    m->deleteLater();
}

static QModelIndex selectedOnView(QTreeView *v)
{
    if (v->selectionModel()->selectedIndexes().isEmpty())
        return v->rootIndex();
    return v->selectionModel()->selectedIndexes().first();
}

void FileSystemManager::menuNewFile()
{
    if (!view->selectionModel())
        return;
    auto m = qobject_cast<QFileSystemModel*>(view->model());
    if (!m)
        return;
    auto idx = selectedOnView(view);
    auto info = m->fileInfo(idx);
    if (!info.isDir()) {
        info = QFileInfo(info.absoluteDir().absolutePath());
    }
    auto fileName = QInputDialog::getText(view->window(), tr("File name"),
                                          tr("Create file on %1")
                                          .arg(info.absoluteFilePath()));
    if (!fileName.isEmpty()) {
        QFile f(QDir(info.absoluteFilePath()).absoluteFilePath(fileName));
        if (!f.open(QFile::WriteOnly)) {
            QMessageBox::critical(view->window(), tr("Error creating file"), f.errorString());
        } else {
            f.close();
            emit requestFileOpen(fileName);
        }
    }
}

void FileSystemManager::menuNewDirectory()
{
    if (!view->selectionModel())
        return;
    auto m = qobject_cast<QFileSystemModel*>(view->model());
    if (!m)
        return;
    auto idx = selectedOnView(view);
    if (!QFileInfo(m->fileInfo(idx)).isDir()) {
        idx = idx.parent();
        if (!m->fileInfo(idx).isDir()) {
            qDebug() << "ERROR parent not a dir";
            return;
        }
    }
    auto name = QInputDialog::getText(view->window(), tr("Folder name"),
                                      tr("Create folder on %1")
                                      .arg(m->fileInfo(idx).absoluteFilePath()));
    if (!name.isEmpty()) {
        qDebug() << "creating" << name << " on " << m->fileName(idx);
        m->mkdir(idx, name);
    }
}

void FileSystemManager::menuNewSymlink()
{
#ifdef Q_OS_UNIX
    if (!view->selectionModel())
        return;
    auto m = qobject_cast<QFileSystemModel*>(view->model());
    if (!m)
        return;
    auto idx = selectedOnView(view);
    if (!QFileInfo(m->fileInfo(idx)).isDir()) {
        idx = idx.parent();
        if (!m->fileInfo(idx).isDir()) {
            qDebug() << "ERROR parent not a dir";
            return;
        }
    }
    QDir targetDir(m->fileInfo(idx).absoluteFilePath());
    QFileDialog dialog(view->window());
    dialog.setWindowTitle(tr("Link target"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(targetDir.absolutePath());
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
        return;
    QFile target(dialog.selectedFiles().first());
    auto linkName = targetDir.absoluteFilePath(QFileInfo(target.fileName()).fileName());
    if (!target.link(linkName))
        QMessageBox::critical(view->window(), tr("Link creation fail"), tr("ERROR: %1").arg(target.errorString()));
#else
    QMessageBox::critical(view->window(), tr("Link creation fail"), tr("ERROR: Not implemented"));
#endif
}

void FileSystemManager::menuItemProperties()
{
    // TODO add property dialog
}

void FileSystemManager::menuItemExecute()
{
    if (!view->selectionModel())
        return;
    auto m = qobject_cast<QFileSystemModel*>(view->model());
    if (!m)
        return;
    auto idx = selectedOnView(view);
    auto info = m->fileInfo(idx);
#ifdef Q_OS_WIN
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
#else
    QProcess::execute(info.absoluteFilePath());
#endif
}

void FileSystemManager::menuItemOpenExternal()
{
    if (!view->selectionModel())
        return;
    auto m = qobject_cast<QFileSystemModel*>(view->model());
    if (!m)
        return;
    auto info = m->fileInfo(selectedOnView(view));
    QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
}

void FileSystemManager::menuItemRename()
{
    view->edit(selectedOnView(view));
}

void FileSystemManager::menuItemDelete()
{
    if (!view->selectionModel())
        return;
    auto m = qobject_cast<QFileSystemModel*>(view->model());
    if (!m)
        return;
    auto items = view->selectionModel()->selectedRows(0);
    QMessageBox msg(view->window());
    msg.setWindowTitle(tr("Delete files"));
    msg.setIcon(QMessageBox::Warning);
    msg.addButton(QMessageBox::Yes);
    msg.addButton(QMessageBox::No);
    if (items.count() > 1) {
        auto forAll = new QCheckBox(tr("Do this operation for all items"), &msg);
        forAll->setCheckState(Qt::Unchecked);
        msg.setCheckBox(forAll);
        msg.addButton(QMessageBox::Cancel);
    }
    int last = -1;
    for(const auto& idx: items) {
        auto parent = idx.parent();
        auto name = m->filePath(idx);
        bool doForAll = false;
        if (msg.checkBox())
            doForAll = (msg.checkBox()->checkState() == Qt::Checked);
        if (!doForAll) {
            msg.setText(tr("Realy remove %1").arg(name));
            last = msg.exec();
        }
        switch(last) {
        case QMessageBox::Yes:
            if (m->fileInfo(idx).isDir()) {
                QDir(m->fileInfo(idx).absoluteFilePath()).removeRecursively();
            } else {
                m->remove(idx);
            }
            view->update(parent);
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
            return;
        }
    }
}

ProjectIconProvider::~ProjectIconProvider()
= default;

FileSystemModel::~FileSystemModel()
= default;
