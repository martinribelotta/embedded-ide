#include "findinfilesdialog.h"
#include "projectview.h"
#include "ui_findinfilesdialog.h"

#include <QtConcurrent>

FindInFilesDialog::FindInFilesDialog(ProjectView *view, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindInFilesDialog),
    projectView(view)
{
    ui->setupUi(this);
    ui->textToFind->addMenuAction(QHash<QString, QString>{
                                      { tr("Regular Expression"), "regex" },
                                      { tr("Case Sensitive"), "case" },
                                      { tr("Wole Words"), "wword" },
                                      { tr("Multiline"), "multi" },
                                  });
    setStatus(tr("Ready"));
    ui->textDirectory->setText(view->projectPath().absolutePath());
    auto watcher = new QFutureWatcher<void>(this);
    watcher->setFuture(future);
    connect(watcher, &QFutureWatcher<void>::finished, [this]() {
        setStatus(tr("Done"));
    });
}

FindInFilesDialog::~FindInFilesDialog()
{
    delete ui;
}

void FindInFilesDialog::on_buttonFind_clicked()
{
    future = QtConcurrent::run([this] () {
        bool isRegex = ui->textToFind->isPropertyChecked("regex");
        bool isCS = ui->textToFind->isPropertyChecked("case");
        // bool isWWord = ui->textToFind->isPropertyChecked("wword");
        bool isMulti = ui->textToFind->isPropertyChecked("multi");
        QRegularExpression re;
        if (isRegex) {
            QRegularExpression::PatternOptions reFlags(QRegularExpression::OptimizeOnFirstUsageOption);
            if (!isCS)
                reFlags.setFlag(QRegularExpression::CaseInsensitiveOption);
            if (isMulti)
                reFlags.setFlag(QRegularExpression::MultilineOption);
            re.setPatternOptions(reFlags);
            re.setPattern(ui->textToFind->text());
        }
        QDirIterator it(ui->textDirectory->text(), QDirIterator::Subdirectories);
        while (it.hasNext()) {
            setStatus(tr("Scanning %1 file").arg(it.next()));
            auto info = it.fileInfo();
            if (info.isFile()) {
                QFile file(info.absoluteFilePath());
                const char *ptr = reinterpret_cast<const char*>(file.map(0, file.size()));
                if (ptr) {
                    QByteArray data = QByteArray::fromRawData(ptr, static_cast<int>(file.size()));
                    QTextCodec *codec = QTextCodec::codecForUtfText(data);
                    QString text = codec->toUnicode(data);
                    if (isRegex) {
                        auto mit = re.globalMatch(text);
                        while (mit.hasNext()) {
                            auto m = mit.next();
                        }
                    } else {

                    }
                }
            }
        }
    });
}

void FindInFilesDialog::setStatus(const QString &message)
{
    ui->labelStatus->setMaximumWidth(ui->labelStatus->width());
    ui->labelStatus->setText(message);
    ui->buttonStop->setEnabled(future.isRunning());
    ui->buttonFind->setDisabled(future.isRunning());
}
