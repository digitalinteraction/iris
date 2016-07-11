#include "handleinput.h"
#include <QPixmap>

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
    list_mac = new QList<List_Map *>();
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
            switch(header->port){
            case TOPO_PACKET:
            {
                unsigned char* list = ((unsigned char *)pack->buffer) + sizeof(struct topo_header);
                qDebug() << header->sizex << header->sizey;
                sizex = header->sizex;
                sizey = header->sizey;
                list_mac->clear();

                for(int i = 0; i < (header->sizex*header->sizey); i++){
                    struct topo_list *item = (struct topo_list *)(list + i*sizeof(struct topo_list));
                    List_Map *temp = new List_Map(item->x, item->y, item->mac);
                    list_mac->append(temp);
                    emit MACChanged(item->x, item->y, item->mac);
                }
                break;
            }
            case IMAGE_PACKET:
            {
                struct low_res_header* low_header = (struct low_res_header*)pack->buffer;
                int posx = 0;
                int posy = 0;

                for(int i = 0; i < list_mac->size(); i++){
                    if(list_mac->at(i) != NULL){
                        //qDebug() << "iterating..." << list_mac->at(i)->mac << low_header->mac;
                        if(list_mac->at(i)->mac == low_header->mac){
                            List_Map * item = (List_Map *)list_mac->at(i);
                            posx = item->x;
                            posy = item->y;
                            //posx = list_mac->at(i)->x;
                            //posy = list_mac->at(i)->y;
                            //if(item->pix != NULL){
                            //    delete item->pix;
                            //}
                            QImage *img = new QImage(((unsigned char *)pack->buffer)+sizeof(struct low_res_header), 400, 30, QImage::Format_RGB888);
                            item->pix = new QPixmap(400,30);
                            item->pix->fromImage(*img);
                            qDebug() << "emitting new image signal";
                            qDebug() << posx << posy << low_header->pos << item->pix;
                            emit NewImageData(posx, posy, item->pix, low_header->pos);

                        }
                    }
                }
                break;
            }
            }

            //free(pack->buffer);
            //free(pack);


        }
        //qDebug("end");

    }
}
