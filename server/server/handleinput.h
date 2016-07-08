#ifndef HANDLEINPUT_H
#define HANDLEINPUT_H
#include "../../network/NetworkControl.h"
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

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
signals:
    void MACChanged(int x, int y, long mac);
};

#endif // HANDLEINPUT_H
