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
#include "appconfig.h"
#include "imageviewer.h"

#include <QImageReader>
#include <QLabel>
#include <QLayout>
#include <QScrollArea>
#include <QToolButton>
#include <QVariant>

template<typename F>
static QToolButton *createButton(const QString& name, QWidget *parent, F& func) {
    auto b = new QToolButton(parent);
    b->setIcon(QIcon(AppConfig::resourceImage({ "actions", name })));
    b->setIconSize(QSize(22, 22));
    b->setAutoRaise(true);
    QObject::connect(b, &QToolButton::clicked, func);
    return b;
}

template<typename F>
static QToolButton *createToggleButton(const QString& name, QWidget *parent, F& func) {
    auto b = new QToolButton(parent);
    b->setIcon(QIcon(AppConfig::resourceImage({ "actions", name })));
    b->setIconSize(QSize(22, 22));
    b->setAutoRaise(true);
    b->setCheckable(true);
    QObject::connect(b, &QToolButton::toggled, func);
    return b;
}

class AspectRatioLabel : public QLabel
{
public:
    explicit AspectRatioLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) :
        QLabel(parent, f) {}
    virtual ~AspectRatioLabel() override;

public slots:
    void setPixmap(const QPixmap& pm) {
        pixmapWidth = pm.width();
        pixmapHeight = pm.height();

        updateMargins();
        QLabel::setPixmap(pm);
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        updateMargins();
        QLabel::resizeEvent(event);
    }


private:
    void updateMargins() {
        if (pixmapWidth <= 0 || pixmapHeight <= 0)
            return;

        int w = this->width();
        int h = this->height();

        if (w <= 0 || h <= 0)
            return;

        if (w * pixmapHeight > h * pixmapWidth)
        {
            int m = (w - (pixmapWidth * h / pixmapHeight)) / 2;
            setContentsMargins(m, 0, m, 0);
        }
        else
        {
            int m = (h - (pixmapHeight * w / pixmapWidth)) / 2;
            setContentsMargins(0, m, 0, m);
        }
    }

    int pixmapWidth = 0;
    int pixmapHeight = 0;
};

AspectRatioLabel::~AspectRatioLabel() = default;

ImageViewer::ImageViewer(QWidget *parent) : QWidget(parent)
{
    auto area = new QScrollArea(this);
    auto label = new AspectRatioLabel(this);
    label->setObjectName("image");
    label->setBackgroundRole(QPalette::Base);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    label->setScaledContents(true);
    area->setWidget(label);
    auto h = new QGridLayout(area);

    setProperty("factor", 1.0);
    auto doScale = [label](double f) { label->resize(label->size() * f); };
    auto zoomIn = [doScale]() { doScale(1.1); };
    auto zoomOut = [doScale]() { doScale(0.99); };
    auto zoomFit = [area, label](bool check) {
        area->setWidgetResizable(check);
        if (!check) {
            label->resize(label->pixmap()->size());
        }
    };

    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    h->addWidget(createToggleButton("zoom-fit-best", area, zoomFit), 0, 1);
    h->addWidget(createButton("zoom-in", area, zoomIn), 0, 2);
    h->addWidget(createButton("zoom-out", area, zoomOut), 0, 3);
    h->setColumnStretch(0, 1);
    h->setColumnStretch(1, 0);
    h->setColumnStretch(2, 0);
    h->setColumnStretch(3, 0);
    h->setColumnStretch(4, 1);
    h->setRowStretch(0, 0);
    h->setRowStretch(1, 1);
    auto l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(area);
}

bool ImageViewer::load(const QString &path)
{
    QPixmap p;
    if (!p.load(path))
        return false;
    auto label = findChild<AspectRatioLabel*>("image");
    label->setPixmap(p);
    label->resize(label->pixmap()->size());
    setProperty("factor", 1.0);
    setWindowFilePath(path);
    return true;
}

class ImageViewerCreator: public IDocumentEditorCreator
{
public:
    ~ImageViewerCreator() override;
    bool canHandleMime(const QMimeType &mime) const override {
        auto supportedMimes = QImageReader::supportedMimeTypes();
        for (const auto& m: supportedMimes)
            if (mime.inherits(m))
                return true;
        return false;
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new ImageViewer(parent);
    }
};

IDocumentEditorCreator *ImageViewer::creator()
{
    return IDocumentEditorCreator::staticCreator<ImageViewerCreator>();
}

ImageViewerCreator::~ImageViewerCreator() = default;
