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

#include <QLabel>
#include <QCheckBox>
#include <QInputDialog>
#include <QMenu>
#include <QProgressBar>
#include <QWidgetAction>
#include <QDesktopServices>
#include <QtConcurrent>
#include <QtDebug>
#include <QPushButton>
#include <QStylePainter>
#include <QShortcut>

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

static QString toHumanReadable(const QString& text) {
    return QString(text).replace('_', ' ');
}

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

class MyButton : public QToolButton {
public:
  explicit MyButton(QWidget* parent = nullptr) : QToolButton(parent) {}
  virtual ~MyButton() {}

  void setPixmap(const QPixmap& pixmap) { m_pixmap = pixmap; }

  virtual QSize sizeHint() const override {
    const auto parentHint = QToolButton::sizeHint();
    // add margins here if needed
    return QSize(parentHint.width() + m_pixmap.width(), std::max(parentHint.height(), m_pixmap.height()));
  }

protected:
  virtual void paintEvent(QPaintEvent* e) override {
    QToolButton::paintEvent(e);

    if (!m_pixmap.isNull()) {
      const int y = (height() - m_pixmap.height()) / 2; // add margin if needed
      QPainter painter(this);
      painter.drawPixmap(5, y, m_pixmap); // hardcoded horizontal margin
    }
  }

private:
  QPixmap m_pixmap;
};

ProjectView::ProjectView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectView)
{
    ui->setupUi(this);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(new QShortcut(QKeySequence(Qt::Key_Delete), ui->treeView), &QShortcut::activated,
            this, &ProjectView::onElementDel);

    labelStatus = new QLabel(ui->targetList);
    QHBoxLayout *lsLayout = new QHBoxLayout(ui->targetList);
    labelStatus->setAttribute( Qt::WA_TransparentForMouseEvents );
    labelStatus->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    lsLayout->addWidget(labelStatus);

    projectButtons += ui->toolButton_export;
    projectButtons += ui->toolButton_find;
    projectButtons += ui->toolButton_refreshProject;
    projectButtons += ui->toolButton_startDebug;

    ui->toolButton_tools->setMenu(createExternalToolsMenu());
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

void ProjectView::setMainMenu(QMenu *m)
{
    ui->toolButton_menu->setMenu(m);
}

void ProjectView::closeProject()
{
    if (ui->treeView->model()) {
        ui->treeView->model()->deleteLater();
        ui->treeView->setModel(nullptr);
    }
    ui->targetList->clear();
    for(auto b: projectButtons) b->setEnabled(false);
}

void ProjectView::openProject(const QString &projectFile)
{
    if (!project().isEmpty())
        closeProject();
    if (!projectFile.isEmpty()) {
        labelStatus->setText(tr("Loading..."));
        QFileInfo mk(projectFile);
        QFileSystemModel *model = new MyFileSystemModel(this);
        model->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Files|QDir::Hidden|QDir::System);
        model->setNameFilterDisables(false);
        model->setNameFilters(QStringList("*"));
        model->setIconProvider(new ProjectIconProvider(this));
        model->setReadOnly(false);

        ui->treeView->model()->deleteLater();
        ui->treeView->setModel(model);
        ui->treeView->setRootIndex(model->setRootPath(mk.path()));
        for(int i=1; i<model->columnCount(); i++)
            ui->treeView->hideColumn(i);
        ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->treeView->header()->hide();
        auto discover = new TargetUpdateDiscover(this);
        connect(discover, SIGNAL(updateFinish(MakefileInfo)), this, SLOT(updateMakefileInfo(MakefileInfo)));
        connect(discover, SIGNAL(updateFinish(MakefileInfo)), discover, SLOT(deleteLater()));
        discover->start(project());
        for(auto b: projectButtons) b->setEnabled(true);
    }
}

void ProjectView::setTargetsViewOn(bool on)
{
    ui->targetList->setEnabled(on);
}

void ProjectView::debugStarted()
{
    ui->toolButton_startDebug->setIcon(QIcon(":/images/actions/media-playback-stop.svg"));
    setProperty("onDebug", true);
}

void ProjectView::debugStoped()
{
    setProperty("onDebug", false);
    ui->toolButton_startDebug->setIcon(QIcon(":/images/actions/debug.svg"));
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
    for(const auto& text: orderedTargets) {
        QString iconName = mapNameToMkIcon.contains(text)? mapNameToMkIcon.value(text) : "run-build";
        QIcon icon = QIcon(QString("://images/actions/%1.svg").arg(iconName));
        QListWidgetItem *item = new QListWidgetItem;
        ui->targetList->addItem(item);
        auto *button = new QPushButton;
        button->setIcon(icon);
        button->setIconSize(QSize(32, 32));
        button->setText(toHumanReadable(text));
        button->setStyleSheet ("text-align: left; padding: 4px;");
        ui->targetList->setItemWidget(item, button);
        item->setSizeHint(button->sizeHint());
        connect(button, &QPushButton::clicked, [text, this](){ emit startBuild(text); });
    }
    // sender()->deleteLater();
    auto ctagProc = new QProcess(this);
    ctagProc->setWorkingDirectory(mk_info.workingDir);
    connect(ctagProc, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            [ctagProc, this] () {
        // Defer execution due to prevent crash in windows while ctagProg is running
        // If this is called during destructor. The single shot is never executed
        QTimer::singleShot(0, [this]() {
            labelStatus->setText(tr("Done with errors"));
            QTimer::singleShot(LAST_MESSAGE_TIMEOUT, labelStatus, &QLabel::clear);
        });
        ctagProc->deleteLater();
    });
    connect(ctagProc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus exitStatus)>(&QProcess::finished),
            [ctagProc, this] (int exitCode, QProcess::ExitStatus exitStatus) {
        qDebug() << "ctags exit code" << exitCode
                 << "ctags exit status" << exitStatus << "\n"
                 << mk_info.tags.parse(ctagProc);
        // Defer execution due to prevent crash in windows while ctagProg is running
        // If this is called during destructor. The single shot is never executed
        QTimer::singleShot(0, [this]() {
            labelStatus->setText(tr("Done"));
            QTimer::singleShot(LAST_MESSAGE_TIMEOUT, labelStatus, &QLabel::clear);
        });
        ctagProc->deleteLater();
    });
    connect(ctagProc, &QProcess::stateChanged, [this](QProcess::ProcessState state) {
        qDebug () << "ctag state " << state;
    });
    labelStatus->setText(tr("Indexing..."));
    ctagProc->start("ctags -R -e --c-kinds=+cdefglmnpstuvx --c++-kinds=+cdefglmnpstuvx -f -");
    emit projectOpened();
}

void ProjectView::on_targetList_clicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->targetList->item(index.row());
    doTarget(item->text());
}

void ProjectView::onDocumentNew()
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

void ProjectView::onFolderNew()
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

void ProjectView::onLinkNew()
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
    QDir targetDir(m->fileInfo(idx).absoluteFilePath());
    QFileDialog dialog(window());
    dialog.setWindowTitle(tr("Link target"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setDirectory(targetDir.absolutePath());
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
        return;
    QFile target(dialog.selectedFiles().first());
    QString linkName(targetDir.absoluteFilePath(QFileInfo(target.fileName()).fileName()));
#ifdef Q_OS_UNIX
    if (!target.link(linkName))
        QMessageBox::critical(window(), tr("Link creation fail"), tr("ERROR: %1").arg(target.errorString()));
#else
    qDebug() << "windows symlink TODO";
#endif
}

void ProjectView::onElementDel()
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
    for(const auto& idx: items) {
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
    if (!project().isEmpty()) {
        auto fileName = QFileDialog::
                getSaveFileName(this,
                                tr("Export file"),
                                tr("Unknown.template"),
                                tr("Tempalte files (*.template *.jtemplate);;"
                                   "Diff files (*.diff);;"
                                   "All files (*)")
                                );
        if (!fileName.isEmpty())
            (new ProjectExporter(
                    fileName,
                    QFileInfo(project()).absolutePath(),
                    parentWidget()->window(),
                    SLOT(actionExportFinish(QString)))
                )->start();
    }
}

static void messageCancelOp(QWidget *w) {
    QMessageBox::information(w->window(), w->tr("Information"), w->tr("Operation canceled"));
}

void ProjectView::toolAction()
{
    QAction *a = qobject_cast<QAction*>(sender());
    if (a) {
        QString text = AppConfig::mutableInstance().filterTextWithVariables(a->data().toString());
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

    for(auto e: QJsonDocument::fromJson(::getenv("EMBEDDED_IDE_TOOLS")).array()) {
        QJsonObject o = e.toObject();
        ProjectView::Entry_t en{ o.value("name").toString(), o.value("command").toString() };
        if (!en.first.isEmpty() && !en.second.isNull() && !list.contains(en))
            list.append(en);
    }

    auto obj = QCoreApplication::instance()->property("hardConf").toJsonObject();
    auto tools = obj.value("tools").toArray();
    for(const QJsonValue& e: tools) {
        auto le = e.toObject();
        ProjectView::Entry_t en{ le.value("name").toString(), le.value("command").toString() };
        if (!en.first.isEmpty() && !en.second.isNull() && !list.contains(en))
            list.append(en);
    }

    return list;
}

QMenu *ProjectView::createExternalToolsMenu()
{
    auto menu = new QMenu(this);
    EntryList_t entries = loadEntries();
    if (!entries.isEmpty()) {
        for(auto e: entries) {
            QString key = e.first;
            QString val = e.second.toString();
            menu->addAction(QIcon(":/images/actions/run-build.svg"), key,
                            this, SLOT(toolAction()))->setData(val);
        }
    } else
        menu->addAction(tr("No entries"))->setDisabled(true);
    menu->setProperty("entries", QVariant::fromValue(entries));
    menu->addSeparator();
    auto *configAction = menu->addAction(QIcon(":/images/configure.svg"), tr("Manage tools"));
    connect(configAction, &QAction::triggered, [this, menu]() {
        ToolManager d(window());
        d.setTools(menu->property("entries").value<EntryList_t>());
        if (d.exec() == QDialog::Accepted)
            ui->toolButton_tools->setMenu(createExternalToolsMenu());
    });
    return menu;
}

#ifdef Q_OS_WIN
#define RUN(fInfo) QDesktopServices::openUrl(QUrl::fromLocalFile(fInfo.absoluteFilePath()))
#else
#define RUN(fInfo) QProcess::execute(fInfo.absoluteFilePath())
#endif

void ProjectView::on_treeView_pressed(const QModelIndex &index)
{
    if (!QApplication::mouseButtons().testFlag(Qt::RightButton))
        return;
    QFileSystemModel *model = qobject_cast<QFileSystemModel*>(ui->treeView->model());
    if (!model)
        return;
    QFileInfo fInfo = model->fileInfo(index);
    auto menu = new QMenu(this);
    auto fileNew = menu->addAction(QIcon(":/images/document-new.svg"), tr("File New"));
    auto folderNew = menu->addAction(QIcon(":/images/folder-new.svg"), tr("Folder New"));
    connect(fileNew, &QAction::triggered, this, &ProjectView::onDocumentNew);
    connect(folderNew, &QAction::triggered, this, &ProjectView::onFolderNew);
#ifdef Q_OS_UNIX
    auto linkNew = menu->addAction(QIcon(":/images/actions/insert-link-symbolic.svg"), tr("New Link"));
    connect(linkNew, &QAction::triggered, this, &ProjectView::onLinkNew);
#endif
    auto editFindAction = menu->addAction(QIcon(":/images/edit-find.svg"), tr("Properties"));
    connect(editFindAction, &QAction::triggered, [this, fInfo]() { fileProperties(fInfo); });
    if (fInfo.isExecutable() && !fInfo.isDir()) {
        auto runAction = menu->addAction(QIcon(":/images/actions/run-build.svg"), tr("Execute"));
        connect(runAction, &QAction::triggered, [fInfo] { RUN(fInfo); });
    }
    auto externalOpenAction = menu->addAction(QIcon(":/images/document-open.svg"), tr("Open External"));
    connect(externalOpenAction, &QAction::triggered, [fInfo] { QDesktopServices::openUrl(QUrl::fromLocalFile(fInfo.absoluteFilePath())); });

    if (ui->treeView->rootIndex() != index) {
        menu->addSeparator();
        auto refreshAction = menu->addAction(QIcon(":/images/actions/view-refresh.svg"), tr("Rename"));
        connect(refreshAction, &QAction::triggered, [this, index]() { ui->treeView->edit(index); });
        auto editDelAction = menu->addAction(QIcon(":/images/edit-delete.svg"), tr("Delete"));
        connect(editDelAction, &QAction::triggered, this, &ProjectView::onElementDel);
    }
    menu->exec(QCursor::pos());
}

void ProjectView::on_toolButton_find_clicked()
{
    emit openFindDialog();
}

void ProjectView::on_toolButton_startDebug_clicked()
{
    emit debugChange(!property("onDebug").toBool());
}

void ProjectView::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    auto idx = ui->treeView->indexAt(pos);
    if (!idx.isValid())
        idx = ui->treeView->rootIndex();
    on_treeView_pressed(idx);
}

void ProjectView::on_toolButton_refreshProject_clicked()
{
    auto saved = project();
    closeProject();
    openProject(saved);
}
