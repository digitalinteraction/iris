#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>
#include <thread>
#include "../../network/NetworkControl.h"
#include "handleinput.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void changeMAC(int x, int y, long mac);
private:
    Ui::MainWindow *ui;
    QGraphicsView *view;
    QGraphicsScene *scene;
    int x;
    int y;
    NetworkControl *nc;
    HandleInput *hi;
    std::thread *net_thread;
    std::thread *con_thread;
};

#endif // MAINWINDOW_H
