#include "markdowneditor.h"

#include <markdownview.h>

#include <QHBoxLayout>
#include <QToolButton>
#include <QGridLayout>
#include <QSplitter>
#include <QScrollBar>

#include <QtConcurrent>
#include <QFutureWatcher>

MarkdownEditor::MarkdownEditor(QWidget *parent): QWidget(parent)
{
    auto l = new QHBoxLayout(this);
    auto s = new QSplitter(Qt::Horizontal, this);
    auto reload = new QToolButton(this);
    renderTimer = new QTimer(this);
    editor = new CodeTextEditor(this);
    view = new MarkdownView(this);
    reload->setIcon(QIcon(":/images/actions/view-refresh.svg"));
    reload->setIconSize(QSize(16, 16));
    view->setCornerWidget(reload);
    s->addWidget(editor);
    s->addWidget(view);
    l->addWidget(s);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    connect(reload, &QToolButton::clicked, this, &MarkdownEditor::updateView);
    connect(editor, &PlainTextEditor::modificationChanged, [this]() {
        notifyModifyObservers();
    });
    connect(editor, &PlainTextEditor::textChanged, [this]() {
        renderTimer->start(1000);
    });
    renderTimer->setSingleShot(true);
    connect(renderTimer, &QTimer::timeout, this, &MarkdownEditor::updateView);
}

bool MarkdownEditor::load(const QString &path) {
    setPath(path);
    auto r = editor->load(path);
    updateView();
    return r;
}

bool MarkdownEditor::save(const QString &path) {
    setWindowFilePath(path);
    return editor->save(path);
}

void MarkdownEditor::setPath(const QString &path) {
    editor->setPath(path);
    view->setWindowFilePath(path);
    view->setSource(QUrl::fromLocalFile(path));
    view->setSearchPaths({ QFileInfo(path).absolutePath() });
    widget()->setWindowFilePath(path);
}

static const QStringList MARKDOWN_EXTENSIONS = { "md" };
static const QStringList MARKDOWN_MIMETYPE = {
    "text/markdown",
    "text/x-markdown",
};

class MarkdownEditorCreator: public IDocumentEditorCreator
{
public:
    ~MarkdownEditorCreator() override;

    static bool in(const QMimeType& t, const QStringList list) {
        for(const auto& mtype: list)
            if (t.inherits(mtype))
                return true;
        return false;
    }

    bool canHandleExtentions(const QStringList &suffixes) const override {
        for(const auto& suffix: suffixes)
            if (MARKDOWN_EXTENSIONS.contains(suffix))
                return true;
        return false;
    }

    bool canHandleMime(const QMimeType &mime) const override {
        if (in(mime, MARKDOWN_MIMETYPE))
            return true;
        return false;
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new MarkdownEditor(parent);
    }
};

MarkdownEditorCreator::~MarkdownEditorCreator() = default;

IDocumentEditorCreator *MarkdownEditor::creator()
{
    return IDocumentEditorCreator::staticCreator<MarkdownEditorCreator>();
}

void MarkdownEditor::updateView()
{
#if 0
    renderTimer->blockSignals(true);
    auto watch = new QFutureWatcher<QString>(this);
    auto text = editor->text();
    auto f = QtConcurrent::run([text]() {
        return MarkdownView::renderHtml(text);
    });
    watch->setFuture(f);
    int pos = view->verticalScrollBar()->value();
    view->clear();
    view->setHtml(tr("<h3>Rendering...</h3>"));
    connect(watch, &QFutureWatcher<QString>::finished, [this, watch, pos]() {
        auto text = watch->future().result();
        view->setHtml(text);
        view->verticalScrollBar()->setValue(pos);
        watch->deleteLater();
        renderTimer->blockSignals(false);
    });
#else
    view->setMarkdown(editor->text());
#endif
}

void MarkdownEditor::closeEvent(QCloseEvent *event)
{
    event->setAccepted(editor->close());
}
