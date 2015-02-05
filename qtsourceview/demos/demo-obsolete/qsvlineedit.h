#ifndef __QSV_LINEEDIT_H__
#define __QSV_LINEEDIT_H__

#include <QLineEdit>
class QPixmap;
class QToolButton;

class QsvLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	QsvLineEdit(QWidget *parent );
	void setIcon(QPixmap i);

protected:
	void resizeEvent(QResizeEvent *);

private slots:	
	void updateEditLine();
private:
	QToolButton *clearButton;	
};

#endif	// __QSV_LINEEDIT_H__
