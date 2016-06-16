#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class QSettings;
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
    void on_buttonBox_accepted();

    void refreshEditor();

    void on_toolButton_clicked();

private:
    Ui::ConfigDialog *ui;
    QSettings *set;
    QsvColorDefFactory *defColors;
    QsvLangDef *langCpp;
    QsvSyntaxHighlighter *syntax;
};

#endif // CONFIGDIALOG_H
