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
    QObject::connect(hi, SIGNAL(NewImageData(int, int, QPixmap *, int)), this, SLOT(changeImageData(int, int, QPixmap *, int)), Qt::QueuedConnection);

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
            //QLabel *img = new QLabel();
            //img->setScaledContents(true);
            //QPixmap pix("/home/tobias/Pictures/a.png");
            //img->setText(QString::number(mac, 16).toUpper());
            //img->setPixmap(pix);

            QGraphicsView * view = new QGraphicsView();
            QGraphicsScene *scene = new QGraphicsScene(0,0,400,30);
            view->setScene(scene);
            layout->addWidget(view);
            //layout->addWidget(img);
        }
        //ui->gridLayout->addWidget(img, x, y);
        ui->gridLayout->addLayout(layout, x,y);
    }else{
        //QLabel *label =(QLabel *) temp->widget();

        //label->setText(QString::number(mac, 16).toUpper());
    }
    qDebug("end");

}

void MainWindow::changeImageData(int posx, int posy, QPixmap* pic, int pos){
    qDebug() << "image slot called" << posx << posy << pos << pic;
    QVBoxLayout *temp = (QVBoxLayout *) ui->gridLayout->itemAtPosition(posx, posy);
    qDebug() << "a";
    if(temp != NULL){
        qDebug() << "b";

        QGraphicsView *view = (QGraphicsView *)temp->itemAt(pos);
        qDebug() << "c";

        if(view != NULL){
            qDebug() << "g";
            if(pic != NULL){
                qDebug() << "h";
                //QGraphicsScene *scene = view->scene();
                qDebug() << "e";
                QGraphicsScene *scene = new QGraphicsScene();
                QPixmap *pix = new QPixmap(*pic);
                QGraphicsPixmapItem *item = new QGraphicsPixmapItem(*pix);

                    qDebug() << "f";
                    qDebug() << item->x() << item->y();
                    scene->addItem(item);
                    qDebug() << "s";
                    view->setScene(scene);
                    //label->setScene(scene);
                    qDebug() << "d";
                qDebug() << "t";

            }
            qDebug() << "e";

        }
        qDebug("g");
    }

}
