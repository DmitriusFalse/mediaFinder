#ifndef LIBRARYITEM_H
#define LIBRARYITEM_H
#include "qdebug.h"
#include <QString>
struct libraryItem {
    QString path;
    QString type;

    friend QDataStream &operator<<(QDataStream &out, const libraryItem &libItem)
    {
        out << libItem.path << libItem.type;
        return out;
    }

    friend QDataStream &operator>>(QDataStream &in, libraryItem &libItem)
    {
        in >> libItem.path >> libItem.type;
        return in;
    }
};
#endif // LIBRARYITEM_H
