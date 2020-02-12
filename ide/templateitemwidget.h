/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TEMPLATEITEMWIDGET_H
#define TEMPLATEITEMWIDGET_H

#include <QFileInfo>
#include <QUrl>
#include <QWidget>

#include <memory>

namespace Ui {
class TemplateItemWidget;
}

class QNetworkAccessManager;

class TemplateItem
{
private:
    QUrl _url;
    QByteArray _data;
    QFileInfo _localFile;
    QByteArray _hash;
public:
    enum class State {
        New,
        Updated,
        Updatable,
        Local,
    };

    TemplateItem() {}
    TemplateItem(const QUrl& u, QByteArray  h);
    TemplateItem(const QFileInfo& local);

    const QUrl& url() const { return _url; }
    void setUrl(const QUrl& u) { _url = u; }
    const QByteArray& data() const { return _data; }
    void setData(const QByteArray& d) { _data = d; }
    const QFileInfo& file() const { return _localFile; }
    void setFile(const QFileInfo& f) { _localFile = f; }
    const QByteArray& hash() const { return _hash; }
    void setHash(const QByteArray& h) { _hash = h; }
    State state() const;
};

Q_DECLARE_METATYPE(TemplateItem)

class TemplateItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TemplateItemWidget(QWidget *parent = nullptr);
    virtual ~TemplateItemWidget();

    void setChecked(bool ck);
    bool isChecked() const;
    const TemplateItem& templateItem() const { return _item; }

public slots:
    void setTemplate(const TemplateItem& item);
    void startDownload(QNetworkAccessManager *net);

signals:
    void downloadStart(const TemplateItem& item);
    void downloadEnd(const TemplateItem& item);
    void downloadMessage(const QString& msg);
    void downloadError(const QString& msg);

private:
    std::unique_ptr<Ui::TemplateItemWidget> ui;
    TemplateItem _item;
};

#endif // TEMPLATEITEMWIDGET_H
