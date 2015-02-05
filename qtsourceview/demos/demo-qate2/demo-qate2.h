/**
 * @brief demo-qate2 is a small demostration of how to Use
 *
 * This example shows how to integrate the qate highlighter into your application.
 * The demo is made as simple as possible (code is embedded inside the heade
 * for simplicity.
 *
 * The magic code is found in the constructor of the class,
 */

// $Id: demo-qate2.h 463 2012-04-07 13:52:45Z diegoiast $

#include <QObject>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QFile>
#include <QIODevice>
#include <QDir>
#include <QToolBar>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

#include "context.h"
#include "highlighter.h"
#include "highlightdefinition.h"
#include "qate/highlightdefinitionmanager.h"
#include "qate/defaultcolors.h"

void load_text(QString fe, QPlainTextEdit *te );
QSharedPointer<TextEditor::Internal::HighlightDefinition> get_highlighter_definition(QString definitionFileName);


class Demo2MainWindow: public QMainWindow {
	Q_OBJECT
protected:
	QPlainTextEdit *textEditor;
	
	Qate::MimeDatabase *mimes;
	Qate::HighlightDefinitionManager *hl_manager;
	TextEditor::Internal::Highlighter *highlighter;
	TextEditor::Internal::Context dummyContext;
	QSharedPointer<TextEditor::Internal::Context> pdummyContext;
	QSharedPointer<TextEditor::Internal::HighlightDefinition>  highlight_definition;

	// this is a hack, otherwise the app crashes
	bool highlightReady;
public:
	Demo2MainWindow(QWidget *parent=NULL);
	void createMainGUI();

public slots:
	void onDefinitionsMetaDataReady();
	void onNew();
	void onOpen();
	bool onSave();
	void onQuit();
	void loadTextFile(QString fileName);
	QString findDefinitionId(const Qate::MimeType &mimeType, bool considerParents) const;
	Qate::MimeType getMimeByExt(const QString &fileName);
};
