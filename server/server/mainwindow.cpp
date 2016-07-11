#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtConcurrent/QtConcurrent>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*for(int i = 0; i < x; i++){
        for(int j = 0; j < y; j++){
            QLabel *img = new QLabel();
            img->setScaledContents(true);
            QPixmap pix("/home/tobias/Pictures/a.png");
            img->setPixmap(pix);
            img->setText("Unknown");
            ui->gridLayout->addWidget(img, i, j);
        }
    }*/


    nc = new NetworkControl();

    hi = new HandleInput();
    //hi->init(in, out);
    hi->init(nc->image_out, nc->image_in);
    QObject::connect(hi, SIGNAL(MACChanged(int, int, long)), this, SLOT(changeMAC(int, int, long)));
    QObject::connect(hi, SIGNAL(NewImageData(int, int, unsigned char *, int, int)), this, SLOT(changeImageData(int, int, unsigned char *, int, int)));

    //QFuture<void> fut = QtConcurrent::run(nc, &NetworkControl::run);
    //QFuture<void> fut2 = QtConcurrent::run(hi, &HandleInput::run);
    net_thread = new std::thread(&NetworkControl::run, nc);
    con_thread = new std::thread(&HandleInput::run, hi);

}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeMAC(int x, int y, long mac){
    qDebug() <<"slot called!" << x << " " << y << " " << mac;
    QLayoutItem *temp = (QLayoutItem *) ui->gridLayout->itemAtPosition(x, y);
    qDebug() << "aa" << temp;
    if(temp == NULL){
        qDebug() << "inserting new object";
        QVBoxLayout *layout = new QVBoxLayout;
        for(int i = 0; i < 10; i++){
            QLabel *img = new QLabel();
            img->setScaledContents(true);
            QPixmap pix("/home/tobias/Pictures/a.png");
            img->setText(QString::number(mac, 16).toUpper());
            img->setPixmap(pix);
            layout->addWidget(img);
        }
        //ui->gridLayout->addWidget(img, x, y);
        ui->gridLayout->addLayout(layout, x,y);
    }else{
        //QLabel *label =(QLabel *) temp->widget();

        //label->setText(QString::number(mac, 16).toUpper());
    }
    qDebug("end");

}

void MainWindow::changeImageData(int posx, int posy, unsigned char *buf, int size, int pos){
    qDebug() << "image slot called" << posx << posy << size << pos;
    QVBoxLayout *temp = (QVBoxLayout *) ui->gridLayout->itemAtPosition(posx, posy);
    qDebug() << "a";
    if(temp != NULL){
        qDebug() << "b";

        QLabel *label = (QLabel *)temp->itemAt(pos);
        qDebug() << "c";

        if(label != NULL){
            qDebug() << "d";

            QImage *img = new QImage(buf, 400, 30, QImage::Format_RGB888);
            qDebug() << "e";

            label->setPixmap(QPixmap::fromImage(*img));
            qDebug() << "f";

        }
    }

}
