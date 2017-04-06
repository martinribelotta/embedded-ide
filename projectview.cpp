#include "projectview.h"
#include "ui_projectview.h"

#include <QFileDialog>
#include <QFileSystemModel>
#include <QStringListModel>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QFuture>

#include <QCheckBox>
#include <QInputDialog>
#include <QMenu>
#include <QProgressBar>
#include <QWidgetAction>
#include <QtConcurrent>
#include <QtDebug>

#include "projecticonprovider.h"
#include "targetupdatediscover.h"
#include "etags.h"
#include "taglist.h"
#include "projectexporter.h"
#include "toolmanager.h"
#include "filepropertiesdialog.h"
#include "appconfig.h"
#include "findinfilesdialog.h"

static const int LAST_MESSAGE_TIMEOUT=1500;

MyFileSystemModel::MyFileSystemModel(QObject *parent) :
    QFileSystemModel(parent)
{
    sectionName.append(tr("Name"));
    sectionName.append(tr("Size"));
}

QVariant MyFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (section < sectionName.size())) {
        return sectionName[section];
    }
    return QFileSystemModel::headerData(section,orientation,role);
}

ProjectView::ProjectView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DocumentView)
{
    ui->setupUi(this);
    ui->tabDebug->setProjectView(this);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    projectButtons += ui->toolButton_documentNew;
    projectButtons += ui->toolButton_export;
    projectButtons += ui->toolButton_elementDel;
    projectButtons += ui->toolButton_folderNew;
    projectButtons += ui->toolButton_find;

#if 0
    QMenu *menu = new QMenu(this);
    QWidgetAction *action = new QWidgetAction(menu);
    tagList = new TagList(this);
    action->setDefaultWidget(tagList);
    menu->addAction(action);
    ui->toolButton_symbols->setMenu(menu);
    ui->toolButton_symbols->hide();
#endif

    ui->tabWidget->setCurrentIndex(0);

    ui->toolButton_tools->setMenu(createExternalToolsMenu());
#ifdef DISABLE_DEBUG_UI
    ui->tabWidget->removeTab(1);
#endif
}

ProjectView::~ProjectView()
{
    delete ui;
}

QString ProjectView::project() const
{
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return QString();
    return m->rootDirectory().absoluteFilePath("Makefile");
}

QString ProjectView::projectName() const
{
    return QFileInfo(projectPath().path()).fileName();
}

QDir ProjectView::projectPath() const
{
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return QDir();
    return m->rootDirectory();
}

DebugInterface *ProjectView::getDebugInterface() const
{
    return ui->tabDebug;
}

void ProjectView::setMainMenu(QMenu *m)
{
    ui->toolButton_menu->setMenu(m);
}

void ProjectView::closeProject()
{
    if (ui->treeView->model()) {
        ui->treeView->model()->deleteLater();
        ui->treeView->setModel(0l);
    }
    ui->targetList->clear();
    foreach(QToolButton *b, projectButtons) b->setEnabled(false);
}

void ProjectView::openProject(const QString &projectFile)
{
    if (!project().isEmpty())
        closeProject();
    if (!projectFile.isEmpty()) {
        //ui->waitSpinner->start();
        ui->labelStatus->setText(tr("Loading..."));
        QFileInfo mk(projectFile);
        QFileSystemModel *model = new MyFileSystemModel(this);
        model->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Files);
        model->setNameFilterDisables(false);
        model->setNameFilters(QStringList("*"));
        model->setIconProvider(new ProjectIconProvider(this));
        model->setReadOnly(false);

        ui->treeView->model()->deleteLater();
        ui->treeView->setModel(model);
        ui->treeView->setRootIndex(model->setRootPath(mk.path()));
        for(int i=2; i<model->columnCount(); i++)
            ui->treeView->hideColumn(i);
        ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        TargetUpdateDiscover *discover = new TargetUpdateDiscover(this);
        connect(discover, SIGNAL(updateFinish(MakefileInfo)), this, SLOT(updateMakefileInfo(MakefileInfo)));
        discover->start(project());
        foreach(QToolButton *b, projectButtons) b->setEnabled(true);
    }
}

void ProjectView::setDebugOn(bool on)
{
    if (on) {
        ui->tabWidget->setCurrentWidget(ui->tabDebug);
    } else {
        ui->tabWidget->setCurrentWidget(ui->tabProject);
    }
}

void ProjectView::on_treeView_activated(const QModelIndex &index)
{
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (m) {
        QFileInfo f = m->fileInfo(index);
        if (f.isFile())
            emit fileOpen(f.filePath());
    }
}

static QHash<QString, QString> mapNameToMkIcon(
{
            // TODO: Add more Makefile targets to icon relations
            { "all", "run-build" },
            { "clean", "run-build-clean" },
            { "clean_all", "run-build-clean" },
            { "erase", "run-build-clean" },
            { "install", "run-build-install-root" },
            { "download", "run-build-install-root" },
            { "program", "run-build-install-root" },
            { "flash", "run-build-install-root" }
});

void ProjectView::updateMakefileInfo(const MakefileInfo &info)
{
    mk_info = info;
    // QIcon icon = QIcon::fromTheme("run-build-configure", QIcon("://images/actions/run-build-configure.svg"));
    ui->targetList->clear();
    QStringList orderedTargets = mk_info.targets;
    orderedTargets.sort();
    foreach(QString t, orderedTargets) {
        QString iconName = mapNameToMkIcon.contains(t)? mapNameToMkIcon.value(t) : "run-build";
        QIcon icon = QIcon(QString("://images/actions/%1.svg").arg(iconName));
        ui->targetList->addItem(new QListWidgetItem(icon, t));
    }
    sender()->deleteLater();
    QProcess *ctagProc = new QProcess(this);
    ctagProc->setWorkingDirectory(mk_info.workingDir);
    connect(ctagProc, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [ctagProc, this] () {
        ui->labelStatus->setText(tr("Done with errors"));
        QTimer::singleShot(LAST_MESSAGE_TIMEOUT, ui->labelStatus, &QLabel::clear);
        ctagProc->deleteLater();
    });
    connect(ctagProc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus exitStatus)>(&QProcess::finished),
            [ctagProc, this] (int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "ctags exit code" << exitCode
                 << "ctags exit status" << exitStatus << "\n"
                 << mk_info.tags.parse(ctagProc);
        // tagList->setTagList(mk_info.tags.all());
        ui->labelStatus->setText(tr("Done"));
        QTimer::singleShot(LAST_MESSAGE_TIMEOUT, ui->labelStatus, &QLabel::clear);
        ctagProc->deleteLater();
    });
    connect(ctagProc, &QProcess::stateChanged, [this](QProcess::ProcessState state) {
        qDebug () << "ctag state " << state;
    });
    ui->labelStatus->setText("Indexing...");
    ctagProc->start("ctags -R -e --c-kinds=+cdefglmnpstuvx --c++-kinds=+cdefglmnpstuvx -f -");
    emit projectOpened();
}

void ProjectView::on_targetList_doubleClicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->targetList->item(index.row());
    emit startBuild(item->text());
}

void ProjectView::on_toolButton_documentNew_clicked()
{
    if (!ui->treeView->selectionModel())
        return;
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return;
    QModelIndex idx = ui->treeView->selectionModel()->selectedIndexes().isEmpty()?
                ui->treeView->rootIndex() :
                ui->treeView->selectionModel()->selectedIndexes().first();
    QFileInfo info = m->fileInfo(idx);
    if (!info.isDir()) {
        info = QFileInfo(info.absoluteDir().absolutePath());
    }
    QString fileName = QInputDialog::getText(this->parentWidget(), tr("File name"),
                                             tr("Create file on %1")
                                                .arg(m->fileInfo(idx).absoluteFilePath()));
    if (!fileName.isEmpty()) {
        QFile f(QDir(info.absoluteFilePath()).absoluteFilePath(fileName));
        if (!f.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, tr("Error creating file"), f.errorString());
        } else {
            f.close();
            emit fileOpen(fileName);
        }
    }
}

void ProjectView::on_toolButton_folderNew_clicked()
{
    if (!ui->treeView->selectionModel())
        return;
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return;
    QModelIndex idx = ui->treeView->selectionModel()->selectedIndexes().isEmpty()?
                ui->treeView->rootIndex():
                ui->treeView->selectionModel()->selectedIndexes().first();
    if (!QFileInfo(m->fileInfo(idx)).isDir()) {
        idx = idx.parent();
        if (!m->fileInfo(idx).isDir()) {
            qDebug() << "ERROR parent not a dir";
            return;
        }
    }
    QString name = QInputDialog::getText(this->parentWidget(), tr("Folder name"),
                                         tr("Create folder on %1")
                                            .arg(m->fileInfo(idx).absoluteFilePath()));
    if (!name.isEmpty()) {
        qDebug() << "creating" << name << " on " << m->fileName(idx);
        m->mkdir(idx, name);
    }
}

void ProjectView::on_toolButton_elementDel_clicked()
{
    if (!ui->treeView->selectionModel())
        return;
    QFileSystemModel *m = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!m)
        return;
    QModelIndexList items = ui->treeView->selectionModel()->selectedRows(0);
    QMessageBox msg(window());
    msg.setWindowTitle(tr("Delete files"));
    msg.setIcon(QMessageBox::Warning);
    msg.addButton(QMessageBox::Yes);
    msg.addButton(QMessageBox::No);
    if (items.count() > 1) {
        QCheckBox *forAll = new QCheckBox(tr("Do this operation for all items"), &msg);
        forAll->setCheckState(Qt::Unchecked);
        msg.setCheckBox(forAll);
        msg.addButton(QMessageBox::Cancel);
    }
    int last = -1;
    foreach(QModelIndex idx, items) {
        QModelIndex parent = idx.parent();
        QString name = m->filePath(idx);
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
            ui->treeView->update(parent);
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
            return;
        }
    }
}

void ProjectView::on_toolButton_export_clicked()
{
    if (!project().isEmpty())
        (new ProjectExporter(
                QFileDialog::
                getSaveFileName(this,
                                tr("Export file"),
                                tr("Unknown.template"),
                                tr("Tempalte files (*.template);;"
                                   "Diff files (*.diff);;"
                                   "All files (*)")
                                ),
                QFileInfo(project()).absolutePath(),
                parentWidget()->window(),
                SLOT(actionExportFinish(QString)))
            )->start();
}

static void messageCancelOp(QWidget *w) {
    QMessageBox::information(w->window(), w->tr("Information"), w->tr("Operation canceled"));
}

void ProjectView::toolAction()
{
    QAction *a = qobject_cast<QAction*>(sender());
    if (a) {
        QString text = a->data().toString();
        text.replace("${{projectpath}}", projectPath().absolutePath());
        text.replace("${{projectname}}", projectName());
        text.replace("${{workspace}}", AppConfig::mutableInstance().defaultApplicationResources());
        text.replace("${{workspace/project}}", AppConfig::mutableInstance().defaultProjectPath());
        text.replace("${{workspace/template}}", AppConfig::mutableInstance().defaultTemplatePath());
        QRegularExpressionMatch m;
        m = QRegularExpression(R"(\${{text: (.+?)}})").match(text);
        if (m.hasMatch()) {
            QString label = m.captured(1);
            bool ok = false;
            QString replacedText = QInputDialog::getText(window(), tr("Input text"), label,
                                                         QLineEdit::Normal, QString(), &ok);
            if (!ok) {
                messageCancelOp(this);
                return;
            }
            text.remove(m.capturedStart(), m.capturedLength());
            text.insert(m.capturedStart(), replacedText);
        }
        m = QRegularExpression(R"(\${{items: (.*?)#(.+?)}})").match(text);
        if (m.hasMatch()) {
            QString label = m.captured(1);
            QStringList items = m.captured(2).split('|');
            bool ok = false;
            QString replacedText = QInputDialog::getItem(window(), tr("Input items"), label,
                                                         items, 0, false, &ok);
            if (!ok) {
                messageCancelOp(this);
                return;
            }
            text.remove(m.capturedStart(), m.capturedLength());
            text.insert(m.capturedStart(), replacedText);
        }
        QString command = text;
        qDebug() << "EXEC" << command;
        emit execTool(command);
    }
}

void ProjectView::fileProperties(const QFileInfo& info)
{
    FilePropertiesDialog d(info, window());
    d.exec();
}

static ProjectView::EntryList_t loadEntries()
{
    ProjectView::EntryList_t list;

    // Settings first, env second
    QSettings sets;
    sets.beginGroup("tools");
    int n = sets.beginReadArray("external");
    for(int i=0; i<n; i++) {
        sets.setArrayIndex(i);
        QString k = sets.value("name").toString();
        QString v = sets.value("command").toString();
        ProjectView::Entry_t e{ k, v };
        if (!k.isEmpty() && !v.isEmpty()) {
            if (!list.contains(e))
                list.append(e);
        }
    }

    QJsonDocument doc = QJsonDocument::fromJson(::getenv("EMBEDDED_IDE_TOOLS"));
    if (doc.isArray()) {
        QJsonArray a = doc.array();
        foreach(QJsonValue e, a) {
            QJsonObject o = e.toObject();
            QString k = o.value("name").toString();
            QString v = o.value("command").toString();
            ProjectView::Entry_t en{ k, v };
            if (!k.isEmpty() && !v.isEmpty()) {
                if (!list.contains(en))
                    list.append(en);
            }
        }
    }
    return list;
}

QMenu *ProjectView::createExternalToolsMenu()
{
    QMenu *menu = new QMenu(this);
    EntryList_t entries = loadEntries();
    if (!entries.isEmpty()) {
        foreach(auto e, entries) {
            QString key = e.first;
            QString val = e.second.toString();
            menu->addAction(QIcon(":/images/actions/run-build.svg"), key,
                            this, SLOT(toolAction()))->setData(val);
        }
    } else
        menu->addAction(tr("No entries"))->setDisabled(true);
    menu->setProperty("entries", QVariant::fromValue(entries));
    menu->addSeparator();
    menu->addAction(QIcon(":/images/configure.svg"), tr("Manage tools"),
                    [this, menu]() {
        ToolManager d(window());
        d.setTools(menu->property("entries").value<EntryList_t>());
        if (d.exec() == QDialog::Accepted)
            ui->toolButton_tools->setMenu(createExternalToolsMenu());
    });
    return menu;
}

void ProjectView::on_treeView_pressed(const QModelIndex &index)
{
    if (!QApplication::mouseButtons().testFlag(Qt::RightButton))
        return;
    QFileSystemModel *model = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!model)
        return;
    QFileInfo fInfo = model->fileInfo(index);
    QMenu *menu = new QMenu(this);
    menu->addAction(QIcon(":/images/edit-find.svg"), tr("Properties"),
                    [this, fInfo]() { fileProperties(fInfo); });
    if (fInfo.isExecutable() && !fInfo.isDir())
        menu->addAction(QIcon(":/images/actions/run-build.svg"), tr("Execute"),
                        [fInfo] { QProcess::execute(fInfo.absoluteFilePath()); });
    menu->addSeparator();
    menu->addAction(QIcon(":/images/actions/view-refresh.svg"), tr("Rename"),
                    [this, index]() { ui->treeView->edit(index); });
    menu->addAction(QIcon(":/images/edit-delete.svg"), tr("Delete"),
                    this, &ProjectView::on_toolButton_elementDel_clicked);
    menu->exec(QCursor::pos());
}

void ProjectView::on_toolButton_find_clicked()
{
    FindInFilesDialog *d = new FindInFilesDialog(this, window());
    d->show();
    connect(d, &QDialog::finished, d, &QObject::deleteLater);
}
