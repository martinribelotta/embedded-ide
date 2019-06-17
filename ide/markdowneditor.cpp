#include "markdowneditor.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QGridLayout>
#include <QSplitter>

#include <QtConcurrent>
#include <QFutureWatcher>

#include <maddy/parser.h>

MarkdownEditor::MarkdownEditor(QWidget *parent): QWidget(parent)
{
    auto l = new QHBoxLayout(this);
    auto s = new QSplitter(Qt::Horizontal, this);
    editor = new CodeTextEditor(this);
    view = new MarkdownView(this);
    auto reload = new QToolButton(view);
    reload->setIcon(QIcon(":/images/actions/view-refresh.svg"));
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
}

bool MarkdownEditor::load(const QString &path) {
    auto r = editor->load(path);
    setWindowFilePath(path);
    updateView();
    return r;
}

bool MarkdownEditor::save(const QString &path) {
    setWindowFilePath(path);
    return editor->save(path);
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
    auto watch = new QFutureWatcher<QString>(this);
    auto text = editor->text();
    auto f = QtConcurrent::run([text]() {
        maddy::Parser p;
        std::stringstream s(text.toStdString());
        return QString::fromStdString(p.Parse(s));
    });
    watch->setFuture(f);
    view->clear();
    view->setHtml(tr("<h3>Rendering...</h3>"));
    connect(watch, &QFutureWatcher<QString>::finished, [this, watch]() {
        view->setHtml(watch->future().result());
        watch->deleteLater();
    });
}

void MarkdownEditor::closeEvent(QCloseEvent *event)
{
    event->setAccepted(editor->close());
}
