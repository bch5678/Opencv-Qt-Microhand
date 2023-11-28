#include "ThreeDrecontrution.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ThreeDrecontrution w;
	w.show();
	return a.exec();
}
