#ifndef IMCOUNTER_WIDGET_H
#define IMCOUNTER_WIDGET_H

#include <QWidget>
#include "ui_imcounter_widget.h"

class imcounter_widget : public QWidget
{
	Q_OBJECT

public:
	imcounter_widget(QWidget *parent = 0);
	~imcounter_widget();

private:
	Ui::imcounter_widget ui;
};

#endif // IMCOUNTER_WIDGET_H
