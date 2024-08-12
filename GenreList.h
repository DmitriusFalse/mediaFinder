#ifndef GENRELIST_H
#define GENRELIST_H
#include "qdebug.h"
#include <QHash>
#include <QString>
struct genre {
    int id;
    QString name;
    QString lang;
};
struct GenreList   {
public:


    QString getGenre(int id){
        auto it = genreMap.find(id);
        if (it != genreMap.end()) {
            return it.value().name;
        } else {
            return "";
        }
        index = genreMap.constBegin();
    }
    GenreList& addGenre(int id, const QString& name, const QString& lang) {
        QString n = name;
        n[0] = n[0].toUpper();
        genreMap.insert(id, genre{id, name, lang});
        index = genreMap.constBegin();
        return *this;
    }

    void resetIterator(){
        index = genreMap.constBegin();  // Сбрасываем итератор
    }
    bool next(){
        // Если текущий итератор не достиг конца
        if (index != genreMap.constEnd()) {
            return true;
        }
        index = genreMap.constBegin(); // Сброс итератора
        return false;
    }
    genre& get(){
        genre& returnValue = const_cast<genre&>(index.value());
        index ++;
        return returnValue;
    }
    uint size(){
        return genreMap.size();
    }
private:
    QMap<uint, genre> genreMap;
    QMap<uint, genre>::const_iterator index;
};
#endif // GENRELIST_H
