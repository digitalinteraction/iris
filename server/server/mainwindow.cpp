#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(QRect(0, 0, 700, 500));
    view = new QGraphicsView(ui->centralWidget);

    QPixmap item_map("/home/tobias/Pictures/a.png");

    item = new QGraphicsPixmapItem();
    item->setPixmap(item_map.scaledToHeight(100, Qt::SmoothTransformation));
    item2 = new QGraphicsPixmapItem(QPixmap("/home/tobias/Pictures/a.png"));
    item3 = new QGraphicsPixmapItem(QPixmap("/home/tobias/Pictures/a.png"));
//get rednering contxt
    button = new QPushButton(ui->centralWidget);
    button->setGeometry(QRect(850,50,50,50));
    button->setText("Push me");
    connect(button, SIGNAL (released()), this, SLOT (handleButton()));



    item->setPos(0,0);
    item2->setPos(100,100);
    item3->setPos(200,200);
    scene->addItem(item);
    scene->addItem(item2);
    scene->addItem(item3);


    view->setScene(scene);
    view->setGeometry(QRect(0, 0, 800, 600));
    view->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handleButton(){
    scene->removeItem(item2);
    scene->removeItem(item);
}

