#include <QTimer>
#include <QDir>
#include <QDialog>
#include <QPushButton>
#include <QFontDialog>

#include "editorconfig.h"
#include "colorsmodel.h"
#include "qsvlangdef.h"
#include "qsvcolordef.h"
#include "qsvlangdeffactory.h"
#include "qsvcolordeffactory.h"
#include "qsvsyntaxhighlighter.h"

#include <QDebug>

#ifdef WIN32
#	define DEFAULT_FONT_NAME "Courier New"
#	define DEFAULT_FONT_SIZE 10
#else
#	define DEFAULT_FONT_NAME "Monospace"
#	define DEFAULT_FONT_SIZE 9
#endif

EditorConfig * EditorConfig::instance = NULL;

EditorConfig::EditorConfig()
{
	dialog = new QDialog;
	ui.setupUi(dialog);

	// set configuration will be done on display	
	colorsModel = NULL;
	currentConfig = getDefaultConfiguration();

	connect( ui.buttonBox	, SIGNAL(clicked(QAbstractButton *))	, this, SLOT(on_buttonBox_clicked(QAbstractButton*)));
	connect( ui.btnChooseFont, SIGNAL(clicked())			, this, SLOT(on_btnChooseFont_clicked()));
	connect( ui.tabWidget	, SIGNAL(currentChanged(int))		, this, SLOT(on_tabWidget_currentChanged(int)));
	connect( ui.colorsCombo	, SIGNAL(currentIndexChanged(int))	, this, SLOT(on_colorsCombo_currentIndexChanged(int)));
}

EditorConfig *EditorConfig::getInstance()
{
	if (EditorConfig::instance == NULL)
		EditorConfig::instance = new EditorConfig;
		
	return EditorConfig::instance;
}

void EditorConfig::showConfigDialog()
{
	// The construction of the syntax highlighter must be postponded
	// to the last possible moment - so the programmer
	// will be able to load the colors directory manually
	// This is also hooked for other initializations
	if (!ui.sampleEdit->getSyntaxHighlighter())
	{
		QsvLangDef *langDefinition = QsvLangDefFactory::getInstanse()->getHighlight("1.cpp");
		currentConfig = this->getDefaultConfiguration();
		ui.sampleEdit->setPlainText(
"#include <stdio.h>\n\
\n\
// main application entry\n\
int main( int argc, char *argv[])\n\
{\n\
	printf(\"hello world!\\n\");\n\
	return 0;\n\
}\n"
		);

		ui.sampleEdit->setSyntaxHighlighter(
			new QsvSyntaxHighlighter( ui.sampleEdit->document(), currentConfig.currentColorScheme, langDefinition ) 
		);
	}
	
	dialog->show();
	ui.sampleEdit->applyConfiguration( currentConfig );
	updateConfiguration();
}

void EditorConfig::closeConfigDialog()
{
	// this is treated as "abort"
	dialog->close();
}

void EditorConfig::loadColorsDirectory( QString directory )
{
	if (directory.isEmpty())
		directory = QDir::currentPath();
	QDir dir(directory, "*.xml");

	QStringList files = dir.entryList(QDir::Files | QDir::NoSymLinks);
	int fileCount =	files.count();

	if (fileCount == 0)
	{
		qDebug( "%s %d - warning: no color definitions found at directory: %s", __FILE__, __LINE__, qPrintable(directory) );
		return;
	}

	for (int i = 0; i < fileCount; ++i)
	{
		QsvColorDefFactory *c = new QsvColorDefFactory( directory + "/" + files[i] );
		colorSchemes << c;
	}
	
	delete colorsModel;
	
	// TODO: how to clean the model...?
	//ui.comboBox->setModel( 0 );
	colorsModel = new ColorsModel( &colorSchemes, this );
	ui.colorsCombo->setModel( colorsModel );
	
	// TODO is this a smart thing...?
	if (currentConfig.currentColorScheme == NULL)
		currentConfig.currentColorScheme = colorSchemes[0];
}


QsvEditorConfigData EditorConfig::getCurrentConfiguration()
{
	return currentConfig;
}

QsvEditorConfigData  EditorConfig::getDefaultConfiguration()
{
	QsvEditorConfigData  defaultConfiguration;
	defaultConfiguration.autoBrackets	= true;
	defaultConfiguration.markCurrentLine	= true;
	defaultConfiguration.showLineNumbers	= true;
	defaultConfiguration.showWhiteSpaces	= true;
	defaultConfiguration.matchBrackets	= true;
	defaultConfiguration.showMargins	= true;
	defaultConfiguration.lineWrapping	= false;
	defaultConfiguration.smartHome		= true;
	defaultConfiguration.insertSpacesInsteadOfTabs = false;
	defaultConfiguration.tabSize		= 8;
	defaultConfiguration.marginsWidth	= 80;
	defaultConfiguration.matchBracketsList	= "()[]{}\"\"''``";
	defaultConfiguration.currentFont	= QFont( DEFAULT_FONT_NAME, DEFAULT_FONT_SIZE );
#ifdef WIN32
	defaultConfiguration.endOfLine = DOS;
#else
	defaultConfiguration.endOfLine = Unix;
#endif
	if (colorSchemes.isEmpty())
		defaultConfiguration.currentColorScheme = NULL;
	else
		defaultConfiguration.currentColorScheme = colorSchemes[0];
		
	return defaultConfiguration;
}

QsvEditorConfigData EditorConfig::getUserConfiguration()
{
	QsvEditorConfigData userConfig;

	userConfig.autoBrackets		= ui.autoBrackets->isChecked();
	userConfig.markCurrentLine	= ui.markCurrentLine->isChecked();
	userConfig.showLineNumbers	= ui.showLineNumbers->isChecked();
	userConfig.showWhiteSpaces	= ui.showWhiteSpaces->isChecked();
	userConfig.showMargins		= ui.showMargins->isChecked();
	userConfig.matchBrackets	= ui.matchBrackets->isChecked();
	userConfig.matchBracketsList	= ui.matchCraketsList->text();
	userConfig.lineWrapping		= ui.wrapLines->isChecked();
	userConfig.insertSpacesInsteadOfTabs	= ui.insertTabs->isChecked();
	userConfig.tabSize		= ui.tabSize->value();
	userConfig.currentFont		= ui.labelFontPreview->font();
	userConfig.smartHome		= ui.useSmartHome->isChecked();
	
	userConfig.tabSize		= ui.tabSize->value();
	userConfig.marginsWidth		= ui.marginSize->value();
	userConfig.matchBracketsList	= ui.matchCraketsList->text();
	userConfig.currentFont		= ui.labelFontPreview->font();

	if (colorSchemes.isEmpty())
		userConfig.currentColorScheme = NULL;
	else
		userConfig.currentColorScheme = colorSchemes[ui.colorsCombo->currentIndex()];
		
	switch (ui.endOfLineComboBox->currentIndex())
	{
		case 0: userConfig.endOfLine = DOS; break;
		case 1: userConfig.endOfLine = Unix; break;
		case 2: userConfig.endOfLine = Mac; break;
		case 3: userConfig.endOfLine = KeepOldStyle; break;
	}
	
	return userConfig;
}

void EditorConfig::applyConfiguration( QsvEditorConfigData c )
{
	currentConfig = c;
}

void EditorConfig::updateConfiguration()
{
	// set the values on the first tab
	ui.autoBrackets->setChecked( currentConfig.autoBrackets );
	ui.markCurrentLine->setChecked( currentConfig.markCurrentLine );
	ui.showLineNumbers->setChecked( currentConfig.showLineNumbers );
	ui.showWhiteSpaces->setChecked( currentConfig.showWhiteSpaces );
	ui.matchBrackets->setChecked( currentConfig.matchBrackets );
	ui.useSmartHome->setChecked( currentConfig.smartHome );
	ui.matchCraketsList->setText( currentConfig.matchBracketsList );
	ui.insertTabs->setChecked( currentConfig.insertSpacesInsteadOfTabs ); 
	ui.tabSize->setValue( currentConfig.tabSize );
	ui.labelFontPreview->setText( currentConfig.currentFont.toString() );
	ui.labelFontPreview->setFont( currentConfig.currentFont );

	int i = colorSchemes.indexOf( currentConfig.currentColorScheme );
	ui.colorsCombo->setCurrentIndex(i);

	// the color configuration is set by this function 	
	ui.sampleEdit->applyConfiguration( currentConfig );

	switch (currentConfig.endOfLine)
	{
		case DOS	:  ui.endOfLineComboBox->setCurrentIndex( 0 ); break;
		case Unix	:  ui.endOfLineComboBox->setCurrentIndex( 1 ); break;
		case Mac	:  ui.endOfLineComboBox->setCurrentIndex( 2 ); break;
		case KeepOldStyle: ui.endOfLineComboBox->setCurrentIndex( 3 ); break;
	}
}

void EditorConfig::loadSettings( QSettings &settings )
{
	QsvEditorConfigData loadedConfig = getDefaultConfiguration();
	
	settings.beginGroup( "QtSourceView" );
	loadedConfig.autoBrackets	= settings.value( "auto-brackets"	, loadedConfig.autoBrackets ).toBool(); 
	loadedConfig.markCurrentLine	= settings.value( "mark-curret-line"	, loadedConfig.markCurrentLine ).toBool();
	loadedConfig.showLineNumbers	= settings.value( "show-line-numbers"	, loadedConfig.showLineNumbers ).toBool();
	loadedConfig.showWhiteSpaces	= settings.value( "show-whitespaces"	, loadedConfig.showWhiteSpaces ).toBool();
	loadedConfig.showMargins	= settings.value( "show-margins"	, loadedConfig.showMargins ).toBool();
	loadedConfig.matchBrackets	= settings.value( "match-brackets"	, loadedConfig.matchBrackets ).toBool();
	loadedConfig.lineWrapping	= settings.value( "line-wrapping"	, loadedConfig.lineWrapping ).toBool();
	loadedConfig.smartHome		= settings.value( "smart-home"		, loadedConfig.smartHome).toBool();
	loadedConfig.insertSpacesInsteadOfTabs	= settings.value( "insert-spaces-instead-of-tabs", loadedConfig.insertSpacesInsteadOfTabs).toBool();
	loadedConfig.tabSize		= settings.value( "tab-size"		, loadedConfig.tabSize).toInt();
	loadedConfig.marginsWidth	= settings.value( "margins-width"	, loadedConfig.marginsWidth).toInt();
	loadedConfig.matchBracketsList	= settings.value( "match-brackets-list"	, loadedConfig.matchBracketsList).toString();
	
	QFont f;
	f.fromString(settings.value( "font", loadedConfig.currentFont).toString());
	loadedConfig.currentFont	= f;

	// TODO finish those 2 entries
	//EndOfLineType		endOfLine;
	//QsvColorDefFactory	*currentColorScheme;
	settings.endGroup();
	
//	applyConfiguration( loadedConfig );
	currentConfig = loadedConfig;
	updateConfiguration();
	emit( configurationModified(loadedConfig) );	
}

void EditorConfig::saveSettings( QSettings &settings )
{
	settings.beginGroup( "QtSourceView" );

	settings.setValue( "auto-brackets"	, currentConfig.autoBrackets ); 
	settings.setValue( "mark-curret-line"	, currentConfig.markCurrentLine );
	settings.setValue( "show-line-numbers"	, currentConfig.showLineNumbers );
	settings.setValue( "show-whitespaces"	, currentConfig.showWhiteSpaces );
	settings.setValue( "show-margins"	, currentConfig.showMargins );
	settings.setValue( "match-brackets"	, currentConfig.matchBrackets );
	settings.setValue( "line-wrapping"	, currentConfig.lineWrapping );
	settings.setValue( "smart-home"		, currentConfig.smartHome);
	settings.setValue( "insert-spaces-instead-of-tabs", currentConfig.insertSpacesInsteadOfTabs);
	settings.setValue( "tab-size"		, currentConfig.tabSize);
	settings.setValue( "margins-width"	, currentConfig.marginsWidth);
	settings.setValue( "match-brackets-list", currentConfig.matchBracketsList);
	settings.setValue( "font"		, currentConfig.currentFont.toString() );

	// TODO finish those 2 entries
	//EndOfLineType		endOfLine;
	//QsvColorDefFactory	*currentColorScheme;
	
	settings.endGroup();
}

void EditorConfig::on_buttonBox_clicked( QAbstractButton * button )
{
	QPushButton  *b = qobject_cast<QPushButton*>(button);
	
	if (!b)
	{
		// this should not happen
		qDebug( "%s %d something funny is happenning", __FILE__, __LINE__ );
		return;
	}
	
	if (b == ui.buttonBox->button(QDialogButtonBox::Ok))
	{
		// set the configuration internally and emit signal, no need to update GUI
		currentConfig = getUserConfiguration();
		dialog->close();
		emit( configurationModified(currentConfig) );
	} 
	else if (b == ui.buttonBox->button(QDialogButtonBox::Apply))
	{
		// set the configuration internally and emit signal
		currentConfig = getUserConfiguration();
		updateConfiguration();
		emit( configurationModified(currentConfig) );
	}
	else if (b == ui.buttonBox->button(QDialogButtonBox::Cancel))
	{
		// lets abort
		dialog->close();
	}
	else if (b == ui.buttonBox->button(QDialogButtonBox::RestoreDefaults))
	{
		// restore default values
		applyConfiguration( getDefaultConfiguration() );
		updateConfiguration();
	}
}

void EditorConfig::on_btnChooseFont_clicked()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, ui.labelFontPreview->font(), dialog );
	
	if (!ok)
		return;
		
	ui.labelFontPreview->setText( font.toString() );
	ui.labelFontPreview->setFont( font );
}

void EditorConfig::on_tabWidget_currentChanged(int index)
{
	if (index != 1)
		return;

	QsvEditorConfigData c = getUserConfiguration();
	ui.sampleEdit->applyConfiguration( c  );
}

void EditorConfig::on_colorsCombo_currentIndexChanged( int index )
{
	Q_UNUSED( index );
	QsvEditorConfigData c = getUserConfiguration();
	
	ui.sampleEdit->applyConfiguration( c );
}

