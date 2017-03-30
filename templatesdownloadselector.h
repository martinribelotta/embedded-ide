#ifndef TEMPLATESDOWNLOADSELECTOR_H
#define TEMPLATESDOWNLOADSELECTOR_H

#include "ui_templatesdownloadselector.h"

#include <vector>

class TemplateWidget;

class Template {
 public:
  enum class ChangeType { None, New, Update };

  Template(const QString &name, const QString &download_url,
           const QString &git_url)
      : name_{name},
        download_url_{download_url},
        git_url_{git_url},
        change_{isNew() ? ChangeType::New : isUpdated() ? ChangeType::Update
                                                        : ChangeType::None} {}
  const QString &name() const;
  ChangeType change() const;
  const QString &download_url() const;
  void setAsIgnored();
  void setAsDownloaded();
  bool isNew() const;

 private:
  bool isUpdated() const;
  bool wasIgnored() const;
  QString uuid() const;
  QString lastDownloadedKey() const;
  QString lastIgnoredKey() const;
  QString lastDownloadedUuid() const;
  QString lastIgnoredUuid() const;

  QString name_;
  QString download_url_;
  QString git_url_;
  ChangeType change_;
};

class TemplatesDownloadSelector : public QDialog {
  Q_OBJECT

 public:
  explicit TemplatesDownloadSelector(const std::vector<Template> &tmpls,
                                     QWidget *parent = 0);
  void addTemplate(const Template &tmpl);

 public slots:
  virtual int exec() override;

 protected:
  virtual void closeEvent(QCloseEvent *event) override;

 private slots:
  void setDonwloadAllEnabled(bool);
  void onDownloadAllSelectedClicked();
  void reenableMultipleOpButtons(QListWidgetItem *);

 private:
  void checkForTemplatesChanges();
  TemplateWidget *nextSelected(TemplateWidget *current);
  void downloadAllSelected(TemplateWidget *current);
  void setUpAmmountLabelsBlinking();
  void setAmmountLabels();

  Ui::TemplatesDownloadSelector ui;
  std::vector<Template> tmpls_;
};

#endif  // TEMPLATESDOWNLOADSELECTOR_H
