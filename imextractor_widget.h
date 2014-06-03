#ifndef IMEXTRACTOR_WIDGET_H
#define IMEXTRACTOR_WIDGET_H

#include <QWidget>
#include "ui_imextractor_widget.h"

class imextractor_widget : public QWidget
{
	Q_OBJECT

public:
	imextractor_widget(QWidget *parent = 0);
	~imextractor_widget();

private:
	Ui::imextractor_widget ui;
};

#endif // IMEXTRACTOR_WIDGET_H
