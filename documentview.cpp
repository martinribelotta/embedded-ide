#include "documentview.h"
#include "ui_documentview.h"

#include <QFileDialog>
#include <QFileSystemModel>
#include <QStringListModel>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QProcess>
#include <QMessageBox>
#include <QDir>

#include <QtDebug>

#include "projecticonprovider.h"
#include "targetupdatediscover.h"
#include "etags.h"

DocumentView::DocumentView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DocumentView)
{
    buildProc = new QProcess(this);
    buildProc->setObjectName("buildProc");
    connect(buildProc, SIGNAL(finished(int)), this, SIGNAL(buildEnd(int)));
    ui->setupUi(this);
    QFile filterFiles(":/build/project-filters.txt");
    filterFiles.open(QFile::ReadOnly);
    while (!filterFiles.atEnd()) {
        QString name = QString(filterFiles.readLine()).remove(':').remove(QRegExp("[\\r\\n]*"));
        QString filter = QString(filterFiles.readLine()).remove(QRegExp("[\\r\\n]*"));
        // qDebug() << "filter" << name << filter;
        if (!name.isEmpty() && !filter.isEmpty())
            ui->filterCombo->addItem(name, filter.split(' '));
        else
            break;
    }
    ui->tabWidget->setCurrentIndex(0);
}

DocumentView::~DocumentView()
{
    delete ui;
}

QString DocumentView::project() const
{
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return QString();
    return m->rootDirectory().absoluteFilePath("Makefile");
}

QDir DocumentView::projectPath() const
{
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return QDir();
    return m->rootDirectory();
}

void DocumentView::closeProject()
{
    if (ui->treeView->model()) {
        ui->treeView->model()->deleteLater();
        ui->treeView->setModel(0l);
    }
    ui->targetList->clear();
}

void DocumentView::openProject(const QString &projectFile)
{
    if (!project().isEmpty())
        closeProject();
    if (!projectFile.isEmpty()) {
        QFileInfo mk(projectFile);
        QFileSystemModel *model = new QFileSystemModel(this);
        model->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Files);
        model->setNameFilterDisables(false);
        model->setNameFilters(QStringList("*"));
        model->setIconProvider(new ProjectIconProvider(this));
        ui->treeView->model()->deleteLater();
        ui->treeView->setModel(model);
        ui->treeView->setRootIndex(model->setRootPath(mk.path()));
        for(int i=2; i<model->columnCount(); i++)
            ui->treeView->hideColumn(i);
#if QT_VERSION >= 0x050000
        ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
        ui->treeView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
#endif
        TargetUpdateDiscover *discover = new TargetUpdateDiscover(this);
        connect(discover, SIGNAL(updateFinish(MakefileInfo)), this, SLOT(updateMakefileInfo(MakefileInfo)));
        discover->start(project());
        ETags::etagsStart(mk.absolutePath(), [this] (const ETags& tags) {
            this->setETags(tags);
        });
    }
}

static QString tmpName() {
    QTemporaryFile f("empty-XXXXXX");
    f.open();
    QString s = QFileInfo(f).fileName();
    f.remove();
    return s;
}

static const QString EXCLUDE_LIST = QString(
        "-x .git* "
        "-x *.o "
        "-x *.d "
        "-x *.elf "
        "-x *.lst "
        "-x *.map "
        "-x .config* "
        "-x *.conf "
        "-x autoconf.h");

QString DocumentView::makeTemplate(const QString &diffFile)
{
    // FIXME Esto tampoco tiene porque estar aca! Pasale el proyecto a otro objeto y fue!
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return tr("No project");
    QString error = tr("Unknow error");
    QDir projectRoot = m->rootDirectory();
    QString tmpDirName = QDir::tempPath() + QDir::separator() + tmpName();
    if (QDir::root().mkdir(tmpDirName)) {
        QProcess diff;
        diff.setWorkingDirectory(projectRoot.absolutePath());
        diff.start(QString("diff -Naur %2 %1 .").arg(tmpDirName).arg(EXCLUDE_LIST));
        if (diff.waitForStarted(3000)) {
            if (diff.waitForFinished(3000)) {
                QString pathText = diff.readAll();
                QFile out(diffFile);
                if (out.open(QFile::WriteOnly)) {
                    out.write(pathText.toLocal8Bit());
                    error = QString();
                } else
                    error = tr("Cant create template file");
            } else
                error = tr("Diff cant finish normaly");
        } else
            error = tr("Diff cant start");
        QDir::root().rmdir(tmpDirName);
    } else
        error = tr("Cant create empty tmp dir");
    return error;
}

void DocumentView::buildStart(const QString &target)
{
    if (buildProc->state() != QProcess::NotRunning)
        buildStop();
    buildProc->setWorkingDirectory(QFileInfo(project()).absoluteDir().absolutePath());
    buildProc->start(QString("make -f \"%1\" \"%2\"")
                     .arg(project())
                     .arg(target));
}

void DocumentView::buildStop()
{
    if (buildProc->state() != QProcess::NotRunning) {
        buildProc->terminate();
        if (!buildProc->waitForFinished(1000)) {
            emit buildStderr(tr("Killing process"));
            buildProc->kill();
        }
    }
}

void DocumentView::setDebugOn(bool on)
{
    if (on) {
        ui->tabWidget->setCurrentWidget(ui->tabDebug);
    } else {
        ui->tabWidget->setCurrentWidget(ui->tabProject);
    }
}

static int passCount = 0;

void DocumentView::setETags(const ETags &tags)
{
    qDebug() << passCount++;
    mk_info.tags = tags;
#if 0
    // Dump ETAGS:
    foreach(QString key, mk_info.tags.tagList()) {
        QList<ETags::Tag> tagsFor = mk_info.tags.find(key);
        qDebug() << "TAG: " << key;
        foreach(ETags::Tag tag, tagsFor) {
            qDebug() << "  cdecl: " << tag.decl;
            qDebug() << "  file:  " << tag.file;
            qDebug() << "  line:  " << tag.line;
        }
    }
#endif
    qDebug() << "tag valid " << (mk_info.tags.isValid());
}

void DocumentView::on_treeView_activated(const QModelIndex &index)
{
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (m) {
        QFileInfo f = m->fileInfo(index);
        if (f.isFile())
            emit fileOpen(f.filePath());
    }
}

void DocumentView::updateMakefileInfo(const MakefileInfo &info)
{
    mk_info = info;
    QIcon icon = QIcon::fromTheme("run-build-configure", QIcon(":/icon-theme/icon-theme/run-build-configure.png"));
    ui->targetList->clear();
    QStringList orderedTargets = mk_info.targets;
    orderedTargets.sort();
    foreach(QString t, orderedTargets)
        ui->targetList->addItem(new QListWidgetItem(icon, t));
    sender()->deleteLater();
    emit projectOpened();
}

void DocumentView::on_targetList_doubleClicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->targetList->item(index.row());
    emit startBuild(item->text());
}

void DocumentView::on_buildProc_readyReadStandardError()
{
    emit buildStderr(buildProc->readAllStandardError());
}

void DocumentView::on_buildProc_readyReadStandardOutput()
{
    emit buildStdout(buildProc->readAllStandardOutput());
}

void DocumentView::on_filterCombo_activated(int idx)
{
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (m)
        m->setNameFilters(ui->filterCombo->itemData(idx).toStringList());
}

void DocumentView::on_filterButton_clicked()
{
}

void DocumentView::on_toolButton_documentNew_clicked()
{
    if (!ui->treeView->selectionModel())
        return;
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return;
    QModelIndex idx = ui->treeView->selectionModel()->selectedIndexes().first();
    QFileInfo info = m->fileInfo(idx);
    if (!info.isDir()) {
        info = QFileInfo(info.absoluteDir().absolutePath());
    }
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("New file"),
                                                    info.absoluteFilePath(),
                                                    tr("C file (*.c);;"
                                                       "C++ file (*.cpp);;"
                                                       "Header (*.h);;"
                                                       "All files (*)"));
    if (!fileName.isEmpty()) {
        QFile f(fileName);
        if (!f.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, tr("Error creating file"), f.errorString());
        } else {
            f.close();
            fileOpen(fileName);
        }
    }
}

void DocumentView::on_toolButton_folderNew_clicked()
{
    if (!ui->treeView->selectionModel())
        return;
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return;
    QModelIndex idx = ui->treeView->selectionModel()->selectedIndexes().first();
    if (!QFileInfo(m->fileInfo(idx)).isDir()) {
        idx = idx.parent();
        if (!m->fileInfo(idx).isDir()) {
            qDebug() << "ERROR parent not a dir";
            return;
        }
    }
    QFileDialog dialog(this->parentWidget());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory(m->fileInfo(idx).absoluteFilePath());
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setLabelText(QFileDialog::FileName, tr("Directory"));
    dialog.setLabelText(QFileDialog::Accept, tr("Create Directory"));
    dialog.setLabelText(QFileDialog::Reject, tr("Cancel"));
    dialog.setLabelText(QFileDialog::LookIn, tr("Look in"));
    dialog.setLabelText(QFileDialog::FileType, tr("Type"));
    if (dialog.exec() == QDialog::Accepted) {
        QString name = QFileInfo(dialog.selectedFiles().first()).fileName();
        qDebug() << "creating" << name << " on " << m->fileName(idx);
        m->mkdir(idx, name);
    }
}

void DocumentView::on_toolButton_elementDel_clicked()
{
    if (!ui->treeView->selectionModel())
        return;
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return;
    QModelIndex idx = ui->treeView->selectionModel()->selectedIndexes().first();
    QString name = m->filePath(idx);
    if (QMessageBox::critical(this->parentWidget(), tr("Confirm"),
                              tr("Realy remove %1").arg(name),
                              QMessageBox::Yes, QMessageBox::No) ==
            QMessageBox::Yes) {
        QModelIndex parent = idx.parent();
        if (m->fileInfo(idx).isDir()) {
            m->rmdir(idx);
        } else {
            m->remove(idx);
        }
        ui->treeView->update(parent);
    }
}
