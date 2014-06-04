#include "imtools.h"
#include <QMimeData>
#include <QDir>

imtools::imtools(QWidget *parent)
	: QMainWindow(parent)
	, _sm(iu::speedup_use_ocl)
	, _mm(iu::match_brute_force)
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

	//ui.txt_left->setAcceptDrops(true);
	//ui.txt_right->setAcceptDrops(true);
	setAcceptDrops(true);

	ui.txt_left->setText(QDir::currentPath());
	ui.txt_right->setText(QDir::currentPath());
}

imtools::~imtools()
{

}

void imtools::lock_ui(bool lock)
{
	ui.tab_diff->setEnabled(!lock);
}

//void imtools::diff()
//{
//	iu::parameters params;
//	params[iu::key_speedup] = _sm;
//	params[iu::key_match_method] = _mm;
//
//	auto w = new imdiff_widget(ui.txt_left->text(), ui.txt_right->text(), params);
//	w->setWindowTitle("diff");
//	w->show();
//	_dlgs.push_back(std::shared_ptr<QWidget>(w));
//}

void imtools::extract()
{

}

void imtools::count()
{

}

void imtools::compare()
{
	lock_ui();

	// get parameters

	emit sig_compare_done();
}

void imtools::compare_proc()
{

}

void imtools::compare_done()
{
	lock_ui(false);
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
	compare();
}

void imtools::right_sel_changed(int index)
{
	compare();
}
