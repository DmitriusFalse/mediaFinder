#ifndef GENRELIST_H
#define GENRELIST_H
#include "qdebug.h"
#include <QHash>
#include <QString>
struct GenreList   {
public:

    struct genre {
        int id;
        QString name;
    };
    QString getGenre(int id){
        auto it = genreMap.find(id);
        if (it != genreMap.end()) {
            return it.value().name;
        } else {
            qDebug() << "Жанр с ID" << id << "не найден.";
            return QString::number (id);
        }
    }
    GenreList& addGenre(int id, const QString& name) {
        genreMap.insert(id, genre{id, name});
        return *this;
    }
private:
    QHash<uint, genre> genreMap;
};
#endif // GENRELIST_H
