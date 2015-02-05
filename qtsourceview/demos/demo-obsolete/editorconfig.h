#ifndef __EDITORCONFIG_H__
#define __EDITORCONFIG_H__

#include <QObject>
#include <QList>
#include <QSettings>
#include "ui_configdialog.h"
#include "qsveditor.h"

class QFont;
class QString;
class QDialog;
class QsvColorDefFactory;
class QsvSyntaxHighlighter;
class ColorsModel;

class EditorConfig : public QObject
{
	Q_OBJECT
	
	EditorConfig();
	
public:
	static EditorConfig *getInstance();
		
	void showConfigDialog();
	void closeConfigDialog();
	void loadColorsDirectory( QString directory );
	
	// gets the currently saved configuration
	// does not modify object status
	QsvEditorConfigData getCurrentConfiguration();
	
	// gets a default set of configuration
	// does not modify object status
	QsvEditorConfigData getDefaultConfiguration();
	
	// gets the configuration which the user is working on, 
	// by reading from the gui
	// does not modify object status
	QsvEditorConfigData getUserConfiguration();
	
	// sets the current configuration to c
	// does not modify the GUI
	void applyConfiguration( QsvEditorConfigData c );
	
	// updates the GUI to refelct the new configuration
	void updateConfiguration();
	
	void loadSettings( QSettings &settings );
	void saveSettings( QSettings &settings );

public slots:
	void on_buttonBox_clicked( QAbstractButton * button );
	void on_btnChooseFont_clicked();
	void on_tabWidget_currentChanged(int index);
	void on_colorsCombo_currentIndexChanged(int index);

signals:
	void configurationModified(QsvEditorConfigData);

private:
	static EditorConfig *instance;
	QList<QsvColorDefFactory*> colorSchemes;
	//QsvSyntaxHighlighter	*highlight;

	QDialog *dialog;
	ColorsModel *colorsModel;
	Ui::ConfigDialog ui;
	QsvEditorConfigData currentConfig;
};

#endif // __EDITORCONFIG_H__
