#include "MainWindow.h"

#include <QApplication>
#include <QMainWindow>
#include <iostream>
using namespace std;

int main(int argc, char* args[]) {
    QApplication* app = new QApplication(argc, args);

    MainWindow* mw = new MainWindow(app);
    mw->show();


    return app->exec();
}
