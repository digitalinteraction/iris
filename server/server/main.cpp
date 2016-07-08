#include "mainwindow.h"
#include <QApplication>
//#include "NetworkControl.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    //myLabel.show();
    return a.exec();
}
