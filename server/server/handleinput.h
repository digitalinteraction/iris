#ifndef HANDLEINPUT_H
#define HANDLEINPUT_H
#include "../../network/NetworkControl.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>
#include "list_map.h"
#include <QList>




class HandleInput : public QWidget
{
    Q_OBJECT
public:
    HandleInput();
    ~HandleInput();
    void init(Packetbuffer *iin, Packetbuffer *oout);
    void run();

private:
    Packetbuffer *in;
    Packetbuffer *out;
    QList<List_Map *> *list_mac;
    uint8_t sizex;
    uint8_t sizey;
signals:
    void MACChanged(int x, int y, long mac);
    void NewImageData(int posx, int posy, QPixmap *pic, int pos);
};

#endif // HANDLEINPUT_H
