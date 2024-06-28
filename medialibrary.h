#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H
#include "settingsdata.h"
#include "workrefreshmovie.h"
#include <QObject>
#include <QString>
struct movieItem{
    int id;
    QString genre;
    QString path;
    QString poster;
    QString name;
    QString library_path;
    QString description;
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

class MediaLibrary : public QObject
{
    Q_OBJECT
public:
    MediaLibrary(QObject *parent, DBManager *dbmanager, SettingsData *settingsData);
    ~MediaLibrary();
    QStringList scanLibraryMovie(QString path);
    void refsreshCollectionMovie();
    movieCollections getMovieInBase(QString detailLevel);
private:
    void handleProgressUpdate(QString str);
    void handleProgressFinish(QStringList str);
    void startScanLibraryMovie();
    void removeOldMoviesInDB();
    SettingsData *m_settingsData;
    DBManager *m_dbmanager;
    WorkRefreshMovie *m_workRMovie;
    QThread *m_workerThread;
    int progress;
signals:
    // void updateProgressBarUI(int i);
    void updateProgressBarUI(QString str);
};

#endif // MEDIALIBRARY_H
