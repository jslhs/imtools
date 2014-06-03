#include "imtools.h"
#include <QMimeData>

imtools::imtools(QWidget *parent)
	: QMainWindow(parent)
	, _sm(iu::speedup_use_ocl)
	, _mm(iu::match_brute_force)
{
	ui.setupUi(this);

	connect(ui.txt_left, SIGNAL(textChanged(const QString&)), this, SLOT(txt_left_changed(const QString&)));
	connect(ui.txt_right, SIGNAL(textChanged(const QString&)), this, SLOT(txt_right_changed(const QString&)));

	connect(ui.btn_diff, SIGNAL(pressed()), this, SLOT(diff()));
	connect(ui.btn_extract, SIGNAL(pressed()), this, SLOT(extract()));
	connect(ui.btn_count, SIGNAL(pressed()), this, SLOT(count()));

	ui.txt_left->setAcceptDrops(true);
	ui.txt_right->setAcceptDrops(true);
	setAcceptDrops(true);
}

imtools::~imtools()
{

}

void imtools::diff()
{
	iu::parameters params;
	params[iu::key_speedup] = _sm;
	params[iu::key_match_method] = _mm;

	auto w = new imdiff_widget(ui.txt_left->text(), ui.txt_right->text(), params);
	w->setWindowTitle("diff");
	w->show();
	_dlgs.push_back(std::shared_ptr<QWidget>(w));
}

void imtools::extract()
{

}

void imtools::count()
{

}

void imtools::opt_cpu(bool checked)
{
	_sm = iu::speedup_default;
}

void imtools::opt_ocl(bool checked)
{
	_sm = iu::speedup_use_ocl;
}

void imtools::opt_cuda(bool checked)
{
	_sm = iu::speedup_use_cuda;
}

void imtools::opt_bf(bool checked)
{
	_mm = iu::match_brute_force;
}

void imtools::opt_flann(bool checked)
{
	_mm = iu::match_flann;
}

void imtools::txt_left_changed(const QString& txt)
{

}

void imtools::txt_right_changed(const QString& txt)
{

}

void imtools::dragEnterEvent(QDragEnterEvent *ev)
{
	if (ev->mimeData()->hasUrls())
	{
		ev->accept();
		ev->setDropAction(Qt::LinkAction);
	}
}

void imtools::dropEvent(QDropEvent *ev)
{
	if (ev->mimeData()->hasUrls())
	{
		ev->accept();
		auto urls = ev->mimeData()->urls();
		if (urls.size() > 0)
			ui.txt_left->setText(urls[0].toLocalFile());
		if (urls.size() > 1)
			ui.txt_right->setText(urls[1].toLocalFile());
	}
}