#ifndef IMDIFF_WIDGET_H
#define IMDIFF_WIDGET_H

#include <QWidget>
#include <QImage>
#include <QLabel>
#include "ui_imdiff_widget.h"
#include "im_utility.h"

#include <thread>

class imdiff_widget : public QWidget
{
	Q_OBJECT

public:
	imdiff_widget(const QString &left, const QString &right, const iu::parameters &params);
	~imdiff_widget();

signals:
	void sig_word_done();

public slots:
	void work_done();

protected:
	void paintEvent(QPaintEvent *ev);

private:
	Ui::imdiff_widget ui;
	QString _left;
	QString _right;
	iu::parameters _params;
	std::thread _wt;
	QImage _img;
	QLabel _img_view;

	void work();
};

#endif // IMDIFF_WIDGET_H
