#include "imtools.h"
#include <QtWidgets/QApplication>
#include <QStyleFactory>

#include "im_utility.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	imtools w;
	w.show();

	//QStyle *style = QStyleFactory::create("fusion");
	//QApplication::setStyle(style);

	return a.exec();
}
