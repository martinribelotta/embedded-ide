#include "templatesdownloadselector.h"

#include <QSettings>
#include <QFileInfo>
#include <QDir>

#include "appconfig.h"
#include "filedownloader.h"

const QString& Template::name() const { return name_; }

Template::ChangeType Template::change() const { return change_; }

const QString& Template::download_url() const { return download_url_; }

void Template::setAsIgnored() {
    QSettings s;
    s.setValue(lastIgnoredKey(), uuid());
}

void Template::setAsDownloaded() {
    QSettings s;
    s.setValue(lastDownloadedKey(), uuid());
}

QString Template::uuid() const {
    QString id{};
    int lastSlashIndex{git_url_.lastIndexOf(QChar{'/'})};
    if (lastSlashIndex != -1) {
        id = git_url_.right((git_url_.size() - 1) - lastSlashIndex);
    }
    return id;
}

bool Template::isNew() const {
    QFileInfo f{AppConfig::mutableInstance().buildTemplatePath(), name_};
    return !f.exists() || (!wasIgnored() && lastDownloadedUuid().isEmpty());
}

bool Template::isUpdated() const {
    return !isNew() && !wasIgnored() && uuid() != lastDownloadedUuid();
}

bool Template::wasIgnored() const { return uuid() == lastIgnoredUuid(); }

QString Template::lastDownloadedKey() const {
    return "templates/" + this->name_ + "/last_downloaded";
}

QString Template::lastIgnoredKey() const {
    return "templates/" + this->name_ + "/last_ignored";
}

QString Template::lastDownloadedUuid() const {
    return QSettings().value(lastDownloadedKey(), "").toString();
}

QString Template::lastIgnoredUuid() const {
    return QSettings().value(lastIgnoredKey(), "").toString();
}

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QToolButton>
#include <QUrl>

class TemplateWidget : public QWidget {
    Q_OBJECT
public:
    explicit TemplateWidget(const QString &style, const Template &t,
                            QListWidgetItem *item2, QWidget *parent = nullptr);
    virtual ~TemplateWidget(){}
    QCheckBox* mutable_selected();
    QToolButton* mutable_download();
    const Template& tmpl() const { return tmpl_; }
    bool isSelected() const { return this->select_->isChecked(); }

signals:
    void downloadFinished(QListWidgetItem*);
    void downloadStart(bool);

public slots:
    void onDownloadClicked();

private:
    QCheckBox* select_;
    QLabel* name_;
    QToolButton* download_;
    QGraphicsOpacityEffect* download_fade_;
    QListWidgetItem* list_widget_item_;
    Template tmpl_;
};

TemplateWidget::TemplateWidget(const QString &style, const Template &t,
                               QListWidgetItem* item, QWidget* parent)
    : QWidget(parent),
      select_{new QCheckBox{this}},
      name_{new QLabel{t.name(), this}},
      download_{new QToolButton{this}},
      download_fade_{new QGraphicsOpacityEffect(parent)},
      list_widget_item_(item),
      tmpl_{t}
{
    QIcon dwn(t.change() == Template::ChangeType::New? ":/images/edit-download.svg" : ":/images/actions/view-refresh.svg");
    download_->setIcon(dwn);
    select_->setChecked(true);
    download_fade_->setOpacity(1.0);
    this->setGraphicsEffect(download_fade_);
    this->setToolTip(name_->text());
    select_->setText("");
    select_->setMaximumWidth(15);
    name_->setMaximumWidth(220);
    name_->setMinimumWidth(220);
    name_->setAlignment(Qt::AlignLeft);
    name_->setStyleSheet(style);
    this->setLayout(new QHBoxLayout{this});
    this->layout()->setSpacing(1);
    this->layout()->addWidget(select_);
    this->layout()->addWidget(name_);
    static_cast<QHBoxLayout*>(this->layout())->addSpacing(120);
    this->layout()->addWidget(download_);
    connect(download_, SIGNAL(clicked()), this, SLOT(onDownloadClicked()));
}

QCheckBox* TemplateWidget::mutable_selected() { return select_; }

QToolButton* TemplateWidget::mutable_download() { return download_; }

void TemplateWidget::onDownloadClicked() {
    this->download_->setEnabled(false);
    this->select_->setEnabled(false);
    emit this->downloadStart(false);
    FileDownloader* downloader = new FileDownloader(this);
    connect(downloader, &FileDownloader::downloadError,
            [downloader, this](const QString& msg) {
        QMessageBox::critical(NULL, QObject::tr("Network error"), msg);
        downloader->deleteLater();
        this->download_->setEnabled(true);
    });
    connect(downloader, &FileDownloader::downloadProgress,
            [this](const QUrl&, int percent) {
        download_fade_->setOpacity(1 - percent / 100.0);
        this->setGraphicsEffect(download_fade_);
    });
    connect(downloader, &FileDownloader::downloadFinished,
            [this](const QUrl&, const QString&) {
        this->setVisible(false);
        const_cast<Template&>(this->tmpl()).setAsDownloaded();
        emit this->downloadFinished(list_widget_item_);
    });
    downloader->startDownload(
                tmpl().download_url(),
                AppConfig::mutableInstance().buildTemplatePath() + "/" + tmpl_.name());
}

void TemplatesDownloadSelector::reenableMultipleOpButtons(QListWidgetItem*) {
    setDonwloadAllEnabled(false);
    ui.checkBox->setEnabled(true);
}

#include "templatesdownloadselector.moc"

#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QTimer>

TemplatesDownloadSelector::TemplatesDownloadSelector(
            const std::vector<Template>& tmpls, QWidget* parent)
    : QDialog(parent), tmpls_{tmpls} {
    ui.setupUi(this);
    for (const auto& tmpl : tmpls) {
        this->addTemplate(tmpl);
    }
    connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(onDownloadAllSelectedClicked()));
    setAmmountLabels();
    setUpAmmountLabelsBlinking();
}

void TemplatesDownloadSelector::setUpAmmountLabelsBlinking() {
    QGraphicsOpacityEffect* newAmmountEffect =
            new QGraphicsOpacityEffect(ui.newAmmount);
    newAmmountEffect->setOpacity(1.0);
    ui.newAmmount->setGraphicsEffect(newAmmountEffect);
    QGraphicsOpacityEffect* updatedAmmountEffect =
            new QGraphicsOpacityEffect(ui.updatedAmmount);
    updatedAmmountEffect->setOpacity(1.0);
    ui.updatedAmmount->setGraphicsEffect(updatedAmmountEffect);
    QTimer* blink = new QTimer(this);
    connect(blink, &QTimer::timeout,
            [this, newAmmountEffect, updatedAmmountEffect]() {
        QPropertyAnimation* animation =
                new QPropertyAnimation(newAmmountEffect, "opacity");
        animation->setDuration(350);
        QPropertyAnimation* animation2 =
                new QPropertyAnimation(updatedAmmountEffect, "opacity");
        animation2->setDuration(350);
        double start = 0.0;
        double end = 1.0;
        static bool visible = false;
        if (!visible) {
            std::swap(start, end);
        }
        visible = !visible;
        animation->setStartValue(start);
        animation->setEndValue(end);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
        animation2->setStartValue(start);
        animation2->setEndValue(end);
        animation2->start(QAbstractAnimation::DeleteWhenStopped);
    });
    blink->start(1000);
}

void TemplatesDownloadSelector::setAmmountLabels() {
    int news{0};
    int updates{0};
    for (int i{0}; i < ui.listWidget->count(); ++i) {
        QListWidgetItem* item{ui.listWidget->item(i)};
        TemplateWidget* tw{
            static_cast<TemplateWidget*>(ui.listWidget->itemWidget(item))};
        if (tw->tmpl().change() == Template::ChangeType::New) {
            ++news;
        } else if (tw->tmpl().change() == Template::ChangeType::Update) {
            ++updates;
        }
    }
    if (news > 0) {
        ui.newAmmount->setText(QString(tr("%1 new")).arg(news));
    } else {
        ui.newAmmount->setText("");
    }
    if (updates > 0) {
        ui.updatedAmmount->setText(QString(tr("%1 updated")).arg(updates));
    } else {
        ui.updatedAmmount->setText("");
    }
}

void TemplatesDownloadSelector::setDonwloadAllEnabled(bool) {
    bool enabled{false};
    for (int i = 0; i < ui.listWidget->count(); ++i) {
        QListWidgetItem* item{ui.listWidget->item(i)};
        TemplateWidget* tw{
            static_cast<TemplateWidget*>(ui.listWidget->itemWidget(item))};
        if (tw->mutable_selected()->isChecked()) {
            enabled = true;
        }
    }
    ui.pushButton->setEnabled(enabled);
}

void TemplatesDownloadSelector::onDownloadAllSelectedClicked() {
    ui.checkBox->setEnabled(false);
    ui.pushButton->setEnabled(false);
    for (int i = 0; i < ui.listWidget->count(); ++i) {
        QListWidgetItem* item{ui.listWidget->item(i)};
        TemplateWidget* tw{
            static_cast<TemplateWidget*>(ui.listWidget->itemWidget(item))};
        disconnect(ui.checkBox, SIGNAL(toggled(bool)), tw->mutable_selected(),
                   SLOT(setChecked(bool)));
        disconnect(tw->mutable_selected(), SIGNAL(toggled(bool)), this,
                   SLOT(setDonwloadAllEnabled(bool)));
        disconnect(tw, SIGNAL(downloadStart(bool)), ui.checkBox,
                   SLOT(setEnabled(bool)));
        disconnect(tw, SIGNAL(downloadStart(bool)), ui.pushButton,
                   SLOT(setEnabled(bool)));
        disconnect(tw, SIGNAL(downloadFinished(QListWidgetItem*)), this,
                   SLOT(reenableMultipleOpButtons(QListWidgetItem*)));
        tw->mutable_selected()->setEnabled(false);
        tw->mutable_download()->setEnabled(false);
    }
    QDir templatePath(QDir::tempPath());
    if (!templatePath.exists()) {
        if (!QDir::root().mkpath(templatePath.absolutePath())) {
            QMessageBox::critical(
                        NULL, QObject::tr("Error"),
                        QObject::tr("Error creating %1").arg(templatePath.absolutePath()));
            return;
        }
    }
    this->downloadAllSelected(nullptr);
}

void TemplatesDownloadSelector::downloadAllSelected(TemplateWidget* current) {
    TemplateWidget *tw{nextSelected(current)};
    if (tw) {
        connect(tw, &TemplateWidget::downloadFinished, [this, tw](QListWidgetItem*) {
            downloadAllSelected(tw);
        });
        tw->onDownloadClicked();
    } else {
        this->close();
    }
}

TemplateWidget* TemplatesDownloadSelector::nextSelected(
            TemplateWidget* current) {
    for (int i = 0; i < ui.listWidget->count(); ++i) {
        QListWidgetItem* item{ui.listWidget->item(i)};
        TemplateWidget* tw{
            static_cast<TemplateWidget*>(ui.listWidget->itemWidget(item))};
        if (current != tw && tw->mutable_selected()->isChecked()) {
            return tw;
        }
    }
    return nullptr;
};

void TemplatesDownloadSelector::addTemplate(const Template& tmpl) {
    QListWidgetItem* item{new QListWidgetItem{}};
    QString style = tmpl.isNew() ? ui.newAmmount->styleSheet()
                                 : ui.updatedAmmount->styleSheet();
    TemplateWidget* tw{new TemplateWidget{style, tmpl, item, this}};
    item->setSizeHint(tw->minimumSizeHint());
    ui.listWidget->addItem(item);
    ui.listWidget->setItemWidget(item, tw);
    QObject::connect(
                tw, &TemplateWidget::downloadFinished, [this](QListWidgetItem* tn) {
        ui.listWidget->itemWidget(tn)->deleteLater();
        delete tn;
        for (int i = 0; i < ui.listWidget->count(); ++i) {
            QListWidgetItem* item{ui.listWidget->item(i)};
            TemplateWidget* tw{
                static_cast<TemplateWidget*>(ui.listWidget->itemWidget(item))};
            connect(tw->mutable_selected(), SIGNAL(toggled(bool)), this,
                    SLOT(setDonwloadAllEnabled(bool)));
            tw->mutable_download()->setEnabled(true);
        }
    });
    QObject::connect(tw, &TemplateWidget::downloadStart, [this](bool) {
        for (int i = 0; i < ui.listWidget->count(); ++i) {
            QListWidgetItem* item{ui.listWidget->item(i)};
            TemplateWidget* tw{
                static_cast<TemplateWidget*>(ui.listWidget->itemWidget(item))};
            disconnect(tw->mutable_selected(), SIGNAL(toggled(bool)), this,
                       SLOT(setDonwloadAllEnabled(bool)));
            tw->mutable_download()->setEnabled(false);
        }
        ui.pushButton->setEnabled(false);
        ui.checkBox->setEnabled(false);
    });
    connect(ui.checkBox, SIGNAL(toggled(bool)), tw->mutable_selected(),
            SLOT(setChecked(bool)));
    connect(tw->mutable_selected(), SIGNAL(toggled(bool)), this,
            SLOT(setDonwloadAllEnabled(bool)));
    connect(tw, SIGNAL(downloadStart(bool)), ui.checkBox, SLOT(setEnabled(bool)));
    connect(tw, SIGNAL(downloadStart(bool)), ui.pushButton,
            SLOT(setEnabled(bool)));
    connect(tw, SIGNAL(downloadFinished(QListWidgetItem*)), this,
            SLOT(reenableMultipleOpButtons(QListWidgetItem*)));
    connect(tw, &TemplateWidget::downloadFinished, [this](){
        this->setAmmountLabels();
    });
}

int TemplatesDownloadSelector::exec() {
    QDir templatePath(AppConfig::mutableInstance().buildTemplatePath());
    if (!templatePath.exists()) {
        if (!QDir::root().mkpath(templatePath.absolutePath())) {
            QMessageBox::critical(this, tr("Error"), tr("Error creating %1")
                                  .arg(templatePath.absolutePath()));
            return QDialog::Rejected;
        }
    }
    return QDialog::exec();
}

void TemplatesDownloadSelector::closeEvent(QCloseEvent* event) {
    for (int i = 0; i < ui.listWidget->count(); ++i) {
        QListWidgetItem* item{ui.listWidget->item(i)};
        TemplateWidget* tw{
            static_cast<TemplateWidget*>(ui.listWidget->itemWidget(item))};
        if (!tw->isSelected()) {
            const_cast<Template&>(tw->tmpl()).setAsIgnored();
        }
    }
    event->accept();
}
