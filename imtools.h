#ifndef IMTOOLS_H
#define IMTOOLS_H

#include <QtWidgets/QMainWindow>
#include "ui_imtools.h"
#include "im_utility.h"

#include <thread>
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
	void sel_left();
	void sel_right();
	void view_opt_changed(int index);
	void opt_show_mp_changed();
	void opt_show_kp_changed();

signals:
	void sig_compare_done();

protected:
	virtual void dragEnterEvent(QDragEnterEvent *ev);
	virtual void dropEvent(QDropEvent *ev);
	virtual void resizeEvent(QResizeEvent *ev);

private:
	QStringList get_image_files(const QString &path);
	void lock_ui(bool lock = true);
	void compare();
	QString sel_folder();
	void compare_proc(const std::string &left, const std::string &right, const iu::parameters &params);
	void show_compare_result();

	enum view_options
	{
		view_show_all
		, view_fit_width
		, view_fit_height
		, view_fit_both
	};
	
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
	QString _left_img_file;
	QString _right_img_file;
	int _ocl_dev;
	iu::matches _mt;
	int _left_img_width;
	
};

#endif // IMTOOLS_H
 
