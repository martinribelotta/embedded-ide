#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class QSettings;
class QProcess;
class QsvColorDefFactory;
class QsvLangDef;
class QsvSyntaxHighlighter;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

private slots:
    void load();

    void save();

    void refreshEditor();

    void on_buttonBox_accepted();

    void on_projectPathSetButton_clicked();

    void on_tbPathAdd_clicked();

    void on_tbPathRm_clicked();

    void on_projectTemplatesPathChange_clicked();

    void on_projectTemplatesDownload_clicked();

private:
    Ui::ConfigDialog *ui;
    QSettings *set;
    QsvColorDefFactory *defColors;
    QsvLangDef *langCpp;
    QsvSyntaxHighlighter *syntax;
};

extern void adjustPath();
extern QString defaultTemplateUrl();
extern QString defaultApplicationResources();
extern QString defaultProjectPath();
extern QString defaultTemplatePath();


#endif // CONFIGDIALOG_H
