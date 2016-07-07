#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QPixmap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handleButton();
private:
    Ui::MainWindow *ui;
    QGraphicsView *view;
    QGraphicsScene *scene;

    QGraphicsPixmapItem *item;
    QGraphicsPixmapItem *item2;
    QGraphicsPixmapItem *item3;
    QPushButton *button;

};

#endif // MAINWINDOW_H
