#ifndef NEWPROJECTFROMREMOTEDIALOG_H
#define NEWPROJECTFROMREMOTEDIALOG_H

#include <QDialog>

#include <memory>

namespace Ui {
class NewProjectFromRemoteDialog;
}

class NewProjectFromRemoteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectFromRemoteDialog(QWidget *parent = nullptr);
    virtual ~NewProjectFromRemoteDialog() override;

    QString projectPath() const;

private slots:
    void startDownload();
    void cancelAll();

signals:
    void cancelRequests();

private:
    Ui::NewProjectFromRemoteDialog *ui;
    struct Priv_t;
    std::unique_ptr<Priv_t> priv;

    enum ProcessStatus {
        NotStarted,
        InProgress,
        FinishOk,
        FinishError,
    };

    void handleArchiveType();
    void setOperationInProgress(ProcessStatus status);
    QString filterOut(const QString& text);
};

#endif // NEWPROJECTFROMREMOTEDIALOG_H
