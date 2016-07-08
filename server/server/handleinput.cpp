#include "handleinput.h"


HandleInput::HandleInput(/*QWidget *parent*/)//:
    //QWidget(parent)
{
    qDebug("HandleInput constructed");
}

/*HandleInput::HandleInput() :
{

}*/

HandleInput::~HandleInput()
{
}

void HandleInput::init(Packetbuffer *iin, Packetbuffer *oout){
    in = iin;
    out = oout;
    qDebug("HandleInput init done");
    qDebug() << this->out << "  " << this->in;
}

void HandleInput::run(){
    //qDebug("start running");
    struct packet * pack = 0;
    while(true){
        //qDebug("run");
        //qDebug() << in << " " << in->signalfd;
        if(in != 0 && in->get(&pack) == 0){
            qDebug("got packet");
            struct topo_header* header = (struct topo_header*)pack->buffer;
            unsigned char* list = ((unsigned char *)pack->buffer) + sizeof(struct topo_header);

            qDebug() << header->sizex << header->sizey;
            for(int i = 0; i < (header->sizex*header->sizey); i++){
                    struct topo_list *item = (struct topo_list *)(list + i*sizeof(struct topo_list));
                    emit MACChanged(item->x, item->y, item->mac);
            }
            free(pack->buffer);
            free(pack);
        }
        //qDebug("end");

    }
}
