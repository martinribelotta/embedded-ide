#include "componentitemwidget.h"
#include "ui_componentitemwidget.h"

ComponentItemWidget::ComponentItemWidget(const CodeTemplate &ct, QWidget *parent) :
    QWidget(parent),
    m_codeTemplate(ct),
    ui(new Ui::ComponentItemWidget)
{
    ui->setupUi(this);
    connect(ui->buttonDownloadNew, &QToolButton::clicked, [this]() { emit downloadItem(m_codeTemplate); });
    connect(ui->buttonRemoveFromFile, &QToolButton::clicked, [this]() { emit removeItem(m_codeTemplate); });
    setPath(ct.path());
    setUrl(ct.url());
}

ComponentItemWidget::~ComponentItemWidget()
{
    delete ui;
}

void ComponentItemWidget::setPath(const QFileInfo &path)
{
    auto& ct = m_codeTemplate;
    ct.setFilePath(path);
    auto name = ct.path().baseName();
    if (name.isEmpty())
        name = QFileInfo(ct.url().fileName()).baseName();
    ui->labelComponentName->setText(name);
}

void ComponentItemWidget::setUrl(const QUrl &url)
{
    auto& ct = m_codeTemplate;
    ct.setUrl(url);
    if (url.isValid()) {
        ui->labelComponentUrl->setText(tr(R"(<a href="%1">%2: %3</a>)")
                                       .arg(ct.url().toString())
                                       .arg(ct.url().host())
                                       .arg(ct.url().fileName()));
        setDownloadable(true);
    } else
        ui->labelComponentUrl->clear();
    ui->checkCanDownload->setEnabled(url.isValid());
    ui->buttonDownloadNew->setEnabled(url.isValid());
}

void ComponentItemWidget::setDownloadable(bool dl)
{
    ui->checkCanDownload->setChecked(dl);
}

bool ComponentItemWidget::isDownloadable() const
{
    return ui->checkCanDownload->isChecked();
}
