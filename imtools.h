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
#include <QSignalMapper>
#include <QLabel>

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
	void left_sel_changed(int index);
	void right_sel_changed(int index);
	//void diff();
	void extract();
	void count();
	void compare_done();
	void ocl_dev_changed(int);

signals:
	void sig_compare_done();

protected:
	virtual void dragEnterEvent(QDragEnterEvent *ev);
	virtual void dropEvent(QDropEvent *ev);

private:
	QStringList get_image_files(const QString &path);
	void lock_ui(bool lock = true);
	void compare();
	void compare_proc(const std::string &left, const std::string &right, const iu::parameters &params);
	
private:
	Ui::imtoolsClass ui;
	iu::speedup_method _sm;
	iu::match_method _mm;
	QStringList _left_imgs;
	QStringList _right_imgs;
	QImage _result_img;
	QLabel _result_view;
	QVector<std::shared_ptr<QWidget>> _dlgs;
	QSignalMapper _signal_mapper;
	QVector<QAction *> _ocl_dev_acts;
	int _ocl_dev;
};

#endif // IMTOOLS_H
 
