#ifndef LIST_MAP_H
#define LIST_MAP_H
#include <cstdint>
#include <QPixmap>

class List_Map
{
public:
    List_Map(int x, int y, uint64_t mac);
    int x;
    int y;
    uint64_t mac;
    QPixmap *pix;
};

#endif // LIST_MAP_H
