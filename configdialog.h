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

private:
    Ui::ConfigDialog *ui;
    QSettings *set;
    QsvColorDefFactory *defColors;
    QsvLangDef *langCpp;
    QsvSyntaxHighlighter *syntax;

    const QString currentStyle() const;
};

#endif // CONFIGDIALOG_H
