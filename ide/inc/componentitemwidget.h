#ifndef COMPONENTITEMWIDGET_H
#define COMPONENTITEMWIDGET_H

#include <QWidget>
#include "codetemplate.h"

namespace Ui {
class ComponentItemWidget;
}

class ComponentItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ComponentItemWidget(const CodeTemplate& ct, QWidget *parent = 0);
    ~ComponentItemWidget();

    void setPath(const QFileInfo& path);
    void setUrl(const QUrl& url);
    void setDownloadable(bool dl);
    bool isDownloadable() const;

signals:
    void downloadItem(const CodeTemplate& item);
    void removeItem(const CodeTemplate& item);

private:
    Ui::ComponentItemWidget *ui;
    CodeTemplate m_codeTemplate;
};

#endif // COMPONENTITEMWIDGET_H
