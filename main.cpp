#include "MainWindow.h"

#include <QApplication>
#include <QMainWindow>
#include <iostream>
using namespace std;

#define DICOM_TAG_VIEWER_VER_MAJOR 1
#define DICOM_TAG_VIEWER_VER_MINOR 0

int main(int argc, char* args[]) {
    QApplication* app = new QApplication(argc, args);

	QString ver = QString::number(DICOM_TAG_VIEWER_VER_MAJOR) + QString(".") + QString::number(DICOM_TAG_VIEWER_VER_MINOR);

    MainWindow* mw = new MainWindow(app);
	mw->setWindowTitle(QString("DICOM Tag Viewer v") + ver);
	mw->setFixedWidth(1200);
	mw->setFixedHeight(900);
    mw->show();


    return app->exec();
}
