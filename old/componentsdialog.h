#ifndef COMPONENTSDIALOG_H
#define COMPONENTSDIALOG_H

#include <QDialog>

namespace Ui {
class ComponentsDialog;
}

class ComponentsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ComponentsDialog(QWidget *parent = 0);
    virtual ~ComponentsDialog();

private slots:
    void on_buttonAddUrl_clicked();

    void on_buttonDelUrl_clicked();

    void on_buttonDownloadAll_clicked();

    void on_buttonUpdateRepos_clicked();

private:
    Ui::ComponentsDialog *ui;

    struct Priv_t;
    Priv_t *d_ptr;
};

#endif // COMPONENTSDIALOG_H
