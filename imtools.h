#ifndef IMTOOLS_H
#define IMTOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_imtools.h"
#include "imdiff_widget.h"
#include "imextractor_widget.h"
#include "imcounter_widget.h"

#include <QVector>
#include <memory>
#include <QDragEnterEvent>
#include <QDropEvent>

class imtools : public QMainWindow
{
	Q_OBJECT

public:
	imtools(QWidget *parent = 0);
	~imtools();

public slots:
	void opt_cpu(bool checked);
	void opt_ocl(bool checked);
	void opt_cuda(bool checked);
	void opt_bf(bool checked);
	void opt_flann(bool checked);
	void txt_left_changed(const QString& txt);
	void txt_right_changed(const QString& txt);
	void diff();
	void extract();
	void count();

protected:
	virtual void dragEnterEvent(QDragEnterEvent *ev);
	virtual void dropEvent(QDropEvent *ev);

private:
	QStringList get_image_files(const QString &path);

private:
	Ui::imtoolsClass ui;
	iu::speedup_method _sm;
	iu::match_method _mm;

	QVector<std::shared_ptr<QWidget>> _dlgs;
};

#endif // IMTOOLS_H
