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
#include "cpptexteditor.h"
#include "filereferencesdialog.h"
#include "icodemodelprovider.h"
#include "textmessagebrocker.h"

#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciabstractapis.h>

#include <QMenu>

#include <QMimeDatabase>
#include <QRegularExpression>
#include <QShortcut>
#include <astyle.h>

#include <QtDebug>
#include <astyle_main.h>

static const QStringList C_CXX_EXTENSIONS = { "c", "cpp", "h", "hpp", "cc", "hh", "hxx", "cxx", "c++", "h++" };
static const QStringList C_MIMETYPE = { "text/x-c++src", "text/x-c++hdr" };
static const QStringList CXX_MIMETYPE = { "text/x-c", "text/x-csrc", "text/x-chdr" };

class MyQsciLexerCPP: public QsciLexerCPP {
protected:
    uint32_t dummy{ 0 };
    mutable QLatin1String keywordList;
public:
    MyQsciLexerCPP(QObject *parent = nullptr, bool caseInsensitiveKeywords = false) :
        QsciLexerCPP(parent, caseInsensitiveKeywords)
    {
        setFoldCompact(false);
    }
    ~MyQsciLexerCPP() override;

    void refreshProperties() override
    {
        QsciLexerCPP::refreshProperties();
        emit propertyChanged("lexer.cpp.track.preprocessor", "0");
    }

    const char *keywords(int set) const override
    {
        //if (set == 5) {
        //    updateKeywordList();
        //    return keywordList.data();
        //} else {
        return QsciLexerCPP::keywords(set);
        //}
    }

private:
    void updateKeywordList() const {
        auto c = qobject_cast<CodeTextEditor*>(editor());
        if (c) {
            // keywordList = c->keywordList();
        }
    }
};

CPPTextEditor::CPPTextEditor(QWidget *parent) : CodeTextEditor(parent)
{
    setProperty("isCXX", false);
    setAutoCompletionSource(AcsNone);
    connect(new QShortcut(QKeySequence("Ctrl+Return"), this), &QShortcut::activated, this, &CPPTextEditor::findReference);
    connect(new QShortcut(QKeySequence("Ctrl+i"), this), &QShortcut::activated, this, &CPPTextEditor::formatCode);
}

CPPTextEditor::~CPPTextEditor()
= default;

bool CPPTextEditor::load(const QString &path)
{
    if (codeModel())
        codeModel()->startIndexingFile(path);
    return CodeTextEditor::load(path);
}

class CPPEditorCreator: public IDocumentEditorCreator
{
public:
    ~CPPEditorCreator() override;

    static bool in(const QMimeType& t, const QStringList list) {
        for(const auto& mtype: list)
            if (t.inherits(mtype))
                return true;
        return false;
    }

    bool canHandleExtentions(const QStringList &suffixes) const override {
        for(const auto& suffix: suffixes)
            if (C_CXX_EXTENSIONS.contains(suffix))
                return true;
        return false;
    }

    bool canHandleMime(const QMimeType &mime) const override {
        if (in(mime, C_MIMETYPE))
            return true;
        if (in(mime, CXX_MIMETYPE))
            return true;
        return false;
    }

    IDocumentEditor *create(QWidget *parent = nullptr) const override {
        return new CPPTextEditor(parent);
    }
};

IDocumentEditorCreator *CPPTextEditor::creator()
{
    return IDocumentEditorCreator::staticCreator<CPPEditorCreator>();
}

void CPPTextEditor::findReference()
{
    if (codeModel()) {
        auto word = wordUnderCursor();

        codeModel()->referenceOf(word, [this, word](const ICodeModelProvider::FileReferenceList& refs)
        {
            FileReferencesDialog d(refs, window());
            connect(&d, &FileReferencesDialog::itemClicked, [this](const QString& path, int line) {
                qDebug() << "open" << path << "at" << line;
                documentManager()->openDocumentHere(path, line, 0);
            });
            d.exec();
        });
    } else
        qDebug() << "No code model defined";
}

static STDCALL char* tempMemoryAllocation(unsigned long memoryNeeded)
{
    char* buffer = new char[memoryNeeded];
    return buffer;
}

static STDCALL void tempError(int errorNumber, const char* errorMessage)
{
    qDebug() << errorNumber << errorMessage;
    TextMessageBrocker::instance().publish("stderrLog", QString("%1: %2").arg(errorNumber).arg(errorMessage));
}

void CPPTextEditor::formatCode()
{
    int l, i;
    getCursorPosition(&l, &i);
    auto inText = selectedText();
    if (inText.isEmpty()) {
        selectAll();
        inText = selectedText();
    }
    auto rawText = inText.toUtf8();
    char* utf8In = rawText.data();
    auto& cfg = AppConfig::instance();
    const auto style = cfg.editorFormatterStyle();
    const auto indentType = cfg.editorTabsToSpaces()? "spaces" : "tab";
    int indentCount = cfg.editorTabWidth();
    auto extraAstyleParams = cfg.editorFormatterExtra();
    char* utf8Out = AStyleMain(utf8In,
                               (QString("--style=%1 --indent=%2=%3 %4")
                                .arg(style)
                                .arg(indentType)
                                .arg(indentCount)
                                .arg(extraAstyleParams)).toLatin1().data(),
                               tempError,
                               tempMemoryAllocation);
    replaceSelectedText(QString::fromUtf8(utf8Out));
    setCursorPosition(l, i);
    ensureLineVisible(l);
}

QMenu *CPPTextEditor::createContextualMenu()
{
    auto menu = CodeTextEditor::createContextualMenu();
    menu->addAction(QIcon(":/images/actions/code-context.svg"),
                    tr("Find Reference"),
                    this, &CPPTextEditor::findReference)
            ->setShortcut(QKeySequence("CTRL+ENTER"));
    return menu;
}

void CPPTextEditor::triggerAutocompletion()
{
    if (codeModel()) {
        int line, index;
        getCursorPosition(&line, &index);
        codeModel()->completionAt(
            ICodeModelProvider::FileReference{ path(), line, index, QString() }, text(),
            [this](const QStringList& completions)
        {
            auto w = wordUnderCursor();
            auto filtered = completions.filter(QRegularExpression(QString(R"(^%1)").arg(w)));
            if (!filtered.isEmpty()) {
                showUserList(1, filtered);
            } else // Fallback autocompletion
                CodeTextEditor::triggerAutocompletion();
        });
    } else {
        CodeTextEditor::triggerAutocompletion();
    }
}

QsciLexer *CPPTextEditor::lexerFromFile(const QString &name)
{
    Q_UNUSED(name);
    setProperty("isCXX", CPPEditorCreator::in(QMimeDatabase().mimeTypeForFile(name), CXX_MIMETYPE));
    return new MyQsciLexerCPP(this);
}

MyQsciLexerCPP::~MyQsciLexerCPP()
= default;

CPPEditorCreator::~CPPEditorCreator()
= default;
