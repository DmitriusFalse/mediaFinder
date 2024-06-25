#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H
#include "settingsdata.h"
#include <QString>
struct movieItem{
    QString name;
    QString fileName;
    QString fileDir;
    QString pathIcon;

};
struct movieCollections {
    QList<movieItem> items;
    int size;
    movieCollections(QList<movieItem>& itemsList) : items(itemsList) {
        size = 0;
    }
    void append(const movieItem &item) {
        items.append(item);
        size++;
    }
    void remove(int index) {
        if (index >= 0 && index < items.size()) {
            items.removeAt(index); // Удаляем элемент по индексу
            size--;
        } else {
            qDebug() << "Ошибка: Неверный индекс для удаления элемента.";
        }
    }
};

class MediaLibrary
{
public:
    MediaLibrary(DBManager *dbmanager, SettingsData *settingsData);
    QStringList scanLibraryMovie(QString path);
    movieCollections getRefsreshCollectionMovie();
private:
    SettingsData *m_settingsData;
    DBManager *m_dbmanager;
};

#endif // MEDIALIBRARY_H
