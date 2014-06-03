#include "imdiff_widget.h"
#include <QPainter>
#include <QPaintEvent>


imdiff_widget::imdiff_widget(const QString &left, const QString &right, const iu::parameters &params)
	: QWidget(nullptr)
	, _left(left)
	, _right(right)
	, _params(params)
	, _img_view("Analyzing, please wait...")
{
	ui.setupUi(this);
	connect(this, SIGNAL(sig_word_done()), this, SLOT(work_done()), Qt::QueuedConnection);
	_wt = std::move(std::thread(std::bind(&imdiff_widget::work, this)));
	ui.scrollArea->setWidget(&_img_view);
}

void imdiff_widget::work()
{
	using namespace iu;
	im_utility u;
	if (_left.isEmpty() || _right.isEmpty()) return;
	auto mt = u.diff(_left.toStdString(), _right.toStdString(), _params);

	QImage img1(_left);
	QImage img2(_right);
	QImage img(img1.width() + img2.width(), std::max(img1.height(), img2.height()), QImage::Format_ARGB32);

	QPainter p(&img);

	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::HighQualityAntialiasing);
	p.drawImage(QPoint(0, 0), img1);
	p.drawImage(QPoint(img1.width(), 0), img2);

	QPen pen;
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(2);
	
	int color = 0;
	for (auto &m : mt)
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

	_img = img;
	
	emit sig_word_done();
	
}

void imdiff_widget::work_done()
{
	double ratio = (double)(_img.width()) / _img.height();
	int width = 1600;
	int height = width / ratio;
	QRect rc(geometry().left(), geometry().top(), width, height);
	setGeometry(rc);
	
	_img_view.setPixmap(QPixmap::fromImage(_img));
	
	update();
}

imdiff_widget::~imdiff_widget()
{
	if (_wt.joinable()) _wt.join();
}

void imdiff_widget::paintEvent(QPaintEvent *ev)
{
	QWidget::paintEvent(ev);
	//QPainter p(ui.view);
	//QRect src(0, 0, _img.width(), _img.height());
	//QRect dst(0, 0, ui.view->geometry().width(), ui.view->geometry().height());
	//p.drawImage(dst, _img, src);
}