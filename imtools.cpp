#include "imtools.h"
#include <QMimeData>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QFileDialog>
#include <QResizeEvent>
#include <chrono>

imtools::imtools(QWidget *parent)
	: QMainWindow(parent)
	, _sm(iu::speedup_use_ocl)
	, _mm(iu::match_brute_force)
	, _ocl_dev(0)
{
	ui.setupUi(this);

	connect(ui.txt_left, SIGNAL(textChanged(const QString&)), this, SLOT(txt_left_changed(const QString&)));
	connect(ui.txt_right, SIGNAL(textChanged(const QString&)), this, SLOT(txt_right_changed(const QString&)));

	//connect(ui.btn_diff, SIGNAL(pressed()), this, SLOT(diff()));
	//connect(ui.btn_extract, SIGNAL(pressed()), this, SLOT(extract()));
	//connect(ui.btn_count, SIGNAL(pressed()), this, SLOT(count()));

	//connect(ui.opt_cpu, SIGNAL(clicked(bool)), this, SLOT(opt_cpu(bool)));
	//connect(ui.opt_ocl, SIGNAL(clicked(bool)), this, SLOT(opt_ocl(bool)));
	//connect(ui.opt_cuda, SIGNAL(clicked(bool)), this, SLOT(opt_cuda(bool)));
	connect(ui.opt_bf, SIGNAL(clicked(bool)), this, SLOT(opt_bf(bool)));
	connect(ui.opt_flann, SIGNAL(clicked(bool)), this, SLOT(opt_flann(bool)));

	connect(ui.left_img_list, SIGNAL(currentIndexChanged(int)), this, SLOT(left_sel_changed(int)));
	connect(ui.right_img_list, SIGNAL(currentIndexChanged(int)), this, SLOT(right_sel_changed(int)));
	connect(ui.view_opt_list, SIGNAL(currentIndexChanged(int)), this, SLOT(view_opt_changed(int)));

	connect(this, SIGNAL(sig_compare_done()), this, SLOT(compare_done()), Qt::QueuedConnection);

	connect(ui.btn_src_left, SIGNAL(pressed()), this, SLOT(sel_left()));
	connect(ui.btn_src_right, SIGNAL(pressed()), this, SLOT(sel_right()));

	connect(ui.opt_show_kp, SIGNAL(clicked()), this, SLOT(opt_show_kp_changed()));
	connect(ui.opt_show_mp, SIGNAL(clicked()), this, SLOT(opt_show_mp_changed()));

	//ui.txt_left->setAcceptDrops(true);
	//ui.txt_right->setAcceptDrops(true);
	setAcceptDrops(true);

	ui.txt_left->setText(QDir::currentPath());
	ui.txt_right->setText(QDir::currentPath());

	ui.result_view->setWidget(&_result_view);

	auto devs = iu::im_utility::ocl_devs();
	for (auto &dev : devs)
	{
		auto act = ui.menu_ocl_devs->addAction(QString(dev.c_str()));
		act->setCheckable(true);
		_signal_mapper.setMapping(act, _ocl_dev_acts.size());
		connect(act, SIGNAL(triggered()), &_signal_mapper, SLOT(map()));
		_ocl_dev_acts.push_back(act);
	}

	if (_ocl_dev_acts.size())
		_ocl_dev_acts[0]->setChecked(true);
	
	connect(&_signal_mapper, SIGNAL(mapped(int)), this, SLOT(ocl_dev_changed(int)));
}

imtools::~imtools()
{

}

QString imtools::sel_folder()
{
	return QFileDialog::getExistingDirectory(this, "Choose Directory");
}

void imtools::sel_left()
{
	auto dir = sel_folder();
	if(!dir.isEmpty()) ui.txt_left->setText(dir);
}

void imtools::sel_right()
{
	auto dir = sel_folder();
	if (!dir.isEmpty()) ui.txt_right->setText(dir);
}

void imtools::ocl_dev_changed(int index)
{
	for (int i = 0; i < _ocl_dev_acts.size(); i++)
	{
		auto act = _ocl_dev_acts[i];
		act->setChecked(i == index);
	}

	_ocl_dev = index;
}

void imtools::lock_ui(bool lock)
{
	ui.tabWidget->setEnabled(!lock);
	//ui.result_view->setEnabled(!lock);
}

void imtools::extract()
{

}

void imtools::count()
{

}

void imtools::compare()
{
	lock_ui();

	_result_view.setPixmap(QPixmap());
	_mt = iu::matches();
	_time_used_ms = 0;

	// get parameters
	int left_idx = ui.left_img_list->currentIndex();
	int right_idx = ui.right_img_list->currentIndex();
	if (!left_idx || !right_idx)
	{
		emit sig_compare_done();
		return;
	}

	//std::string left, right;
	using namespace iu;
	parameters params;

	auto left = _left_img_file.toStdString();
	auto right = _right_img_file.toStdString();
	if (!QFile::exists(left.c_str()) || !QFile::exists(right.c_str()))
	{
		emit sig_compare_done();
		return;
	}
	params[key_speedup] = speedup_use_ocl;
	params[key_match_method] = _mm;
	params[key_ocl_dev] = _ocl_dev;

	_result_view.setText("Analyzing, please wait...");
	auto t = std::thread([&, left, right, params](){ compare_proc(left, right, params); });
	t.detach();
}

void imtools::compare_proc(const std::string &left, const std::string &right, const iu::parameters &params)
{
	using namespace iu;
	using namespace std;
	using namespace chrono;

	im_utility u;
	auto t0 = high_resolution_clock::now();
	_mt = u.diff(left, right, params, &_left_kps, &_right_kps);
	auto t = high_resolution_clock::now() - t0;
	_time_used_ms = duration_cast<milliseconds>(t).count();
	emit sig_compare_done();
}

void imtools::compare_done()
{
	QImage img1(_left_img_file);
	QImage img2(_right_img_file);
	QImage img(img1.width() + img2.width(), std::max(img1.height(), img2.height()), QImage::Format_ARGB32);

	//if (img1.isNull() && img2.isNull()) return;
	_left_img_width = img1.width();

	QPainter p(&img);
	
	p.drawImage(QPoint(0, 0), img1);
	p.drawImage(QPoint(_left_img_width, 0), img2);

	_result_img = img;

	show_compare_result();
	lock_ui(false);
}

void imtools::show_compare_result()
{
	auto img = _result_img;
	QPainter p(&img);
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::HighQualityAntialiasing);

	if (ui.opt_show_mp->isChecked())
	{
		QPen pen;
		pen.setStyle(Qt::SolidLine);
		pen.setWidth(2);

		int color = 0;
		for (auto &m : _mt)
		{
			QColor c(color);
			c.setAlpha(150);
			pen.setColor(c);
			p.setPen(pen);
			auto pt1 = QPointF(m.pt1.x, m.pt1.y);
			auto pt2 = QPointF(m.pt2.x + _left_img_width, m.pt2.y);
			QPainterPath path;
			path.addEllipse(pt1, 5, 5);
			path.addEllipse(pt2, 5, 5);
			p.fillPath(path, QBrush(QColor(255, 0, 0, 75)));
			p.drawLine(pt1, pt2);
			p.drawEllipse(pt1, m.pt1.size / 2.0, m.pt1.size / 2.0);
			p.drawEllipse(pt2, m.pt2.size / 2.0, m.pt2.size / 2.0);
			color += 1024;
		}
	}

	if (ui.opt_show_kp->isChecked())
	{
		auto draw_kp = [](QPainter &p, const iu::key_point &kp, int xoff){
			double w = kp.size, h = w;
			QRectF rc(kp.x - w / 2.0 + xoff, kp.y - h / 2.0, w, h);
			p.drawRect(rc);
		};

		p.setPen(QColor(255, 0, 0));

		for (auto &kp : _left_kps)
		{
			draw_kp(p, kp, 0);
		}

		for (auto &kp : _right_kps)
		{
			draw_kp(p, kp, _left_img_width);
		}
	}

	auto g = ui.result_view->geometry();
	auto w = g.width() - 2;
	auto h = g.height() - 20;

	QImage scaled_img;
	view_options view_opt = static_cast<view_options>(ui.view_opt_list->currentIndex());
	switch (view_opt)
	{
	case view_show_all:
		scaled_img = img;
		break;
	case view_fit_height:
		scaled_img = img.scaledToHeight(h);
		break;
	case view_fit_width:
		scaled_img = img.scaledToWidth(w);
		break;
	case view_fit_both:
		scaled_img = img.scaledToHeight(h);
		scaled_img = scaled_img.scaledToWidth(w);
		break;
	}

	QPainter ps(&scaled_img);
	ps.setPen(QColor(255, 255, 255));
	QString str_time, str_kp, str_match;
	str_time.sprintf("Time Used: %.2fs", _time_used_ms / 1000.0);
	str_match.sprintf("Match Points: %d, Match Rate: %.2f%%", _mt.size(), _mt.size() * 100.0 / (double)_right_kps.size());
	str_kp.sprintf("Key Points: %d / %d", _left_kps.size(), _right_kps.size());
	ps.drawText(QPoint(0, 15), str_time);
	ps.drawText(QPoint(0, 30), str_match);
	ps.drawText(QPoint(0, 45), str_kp);

	_result_view.setPixmap(QPixmap::fromImage(scaled_img));
	
}

void imtools::opt_cpu(bool checked)
{
	if(checked) _sm = iu::speedup_default;
	compare();
}

void imtools::opt_ocl(bool checked)
{
	if (checked) _sm = iu::speedup_use_ocl;
	compare();
}

void imtools::opt_cuda(bool checked)
{
	if (checked) _sm = iu::speedup_use_cuda;
	compare();
}

void imtools::opt_bf(bool checked)
{
	if (checked) _mm = iu::match_brute_force;
	compare();
}

void imtools::opt_flann(bool checked)
{
	if (checked) _mm = iu::match_flann;
	compare();
}

void imtools::txt_left_changed(const QString& txt)
{
	_left_imgs = get_image_files(txt);
	ui.left_img_list->clear();
	_left_imgs.push_front("Select...");
	ui.left_img_list->addItems(_left_imgs);
}

void imtools::txt_right_changed(const QString& txt)
{
	_right_imgs = get_image_files(txt);
	ui.right_img_list->clear();
	_right_imgs.push_front("Select...");
	ui.right_img_list->addItems(_right_imgs);
}

QStringList imtools::get_image_files(const QString &path)
{
	QDir d(path);
	QStringList filters;
	filters << "*.jpg" << "*.jpeg" << "*.png" << "*.tiff" << "*.bmp";
	d.setNameFilters(filters);
	return d.entryList(QDir::Files);
}

void imtools::dragEnterEvent(QDragEnterEvent *ev)
{
	if (ev->mimeData()->hasUrls())
	{
		ev->accept();
		
		bool can_drop = false;
		for (auto &url : ev->mimeData()->urls())
		{
			QDir d(url.toLocalFile());
			if (d.exists())
			{
				can_drop = true;
				break;
			}
		}

		ev->setDropAction(can_drop ? Qt::LinkAction : Qt::IgnoreAction);
	}
}

void imtools::dropEvent(QDropEvent *ev)
{
	if (ev->mimeData()->hasUrls())
	{
		ev->accept();
		auto urls = ev->mimeData()->urls();
		if (urls.size() == 1)
		{
			ui.txt_left->setText(urls[0].toLocalFile());
			ui.txt_right->setText(urls[0].toLocalFile());
		}
		else if (urls.size() > 1)
		{
			ui.txt_left->setText(urls[0].toLocalFile());
			ui.txt_right->setText(urls[1].toLocalFile());
		}
		compare();
	}
}

void imtools::left_sel_changed(int index)
{
	_left_img_file = ui.txt_left->text() + "/" + ui.left_img_list->itemText(index);
	compare();
}

void imtools::right_sel_changed(int index)
{
	_right_img_file = ui.txt_right->text() + "/" + ui.right_img_list->itemText(index);
	compare();
}

void imtools::view_opt_changed(int index)
{
	show_compare_result();
}

void imtools::resizeEvent(QResizeEvent *ev)
{
	show_compare_result();
}

void imtools::opt_show_mp_changed()
{
	show_compare_result();
}

void imtools::opt_show_kp_changed()
{
	show_compare_result();
}