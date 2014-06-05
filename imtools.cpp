#include "imtools.h"
#include <QMimeData>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QFileDialog>

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

	connect(this, SIGNAL(sig_compare_done()), this, SLOT(compare_done()), Qt::QueuedConnection);

	connect(ui.btn_src_left, SIGNAL(pressed()), this, SLOT(sel_left()));
	connect(ui.btn_src_right, SIGNAL(pressed()), this, SLOT(sel_right()));

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
	im_utility u;
	_mt = u.diff(left, right, params);
	
	emit sig_compare_done();
}

void imtools::compare_done()
{
	show_compare_result();
	lock_ui(false);
}

void imtools::show_compare_result()
{
	QImage img1(_left_img_file);
	QImage img2(_right_img_file);
	QImage img(img1.width() + img2.width(), std::max(img1.height(), img2.height()), QImage::Format_ARGB32);
	
	if (img1.isNull() && img2.isNull()) return;

	QPainter p(&img);
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::HighQualityAntialiasing);
	p.drawImage(QPoint(0, 0), img1);
	p.drawImage(QPoint(img1.width(), 0), img2);

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
		auto pt2 = QPointF(m.pt2.x + img1.width(), m.pt2.y);
		QPainterPath path;
		path.addEllipse(pt1, 5, 5);
		path.addEllipse(pt2, 5, 5);
		p.fillPath(path, QBrush(QColor(255, 0, 0, 75)));
		p.drawLine(pt1, pt2);
		p.drawEllipse(pt1, m.pt1.size, m.pt1.size);
		p.drawEllipse(pt2, m.pt2.size, m.pt2.size);
		color += 1024;
	}

	_result_view.setPixmap(QPixmap::fromImage(img));
	
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
