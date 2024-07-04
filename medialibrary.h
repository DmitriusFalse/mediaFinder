#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H
#include "qnetworkreply.h"
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
        this->size=items.size ();
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

struct EpisodeInfo {
    int ID;
    int episodeNumber;
    int seasonsNumber;
    QString episodeTitle;
    QString filePath;
};

// struct SeasonInfo {
//     int seasonNumber;
//     QDate releaseDate;
//     // QList<EpisodeInfo> episodes;
//     QMap<int, EpisodeInfo> episodes;
// };

struct ShowInfo {
    int ID;
    QString seriesName;
    QString genre;
    QString description;
    QString posterPath;
    QString libraryPath;
    int totalSeasons;
    // QList<SeasonInfo> seasons;
    // QMap<int, QList<EpisodeInfo>> Episodes;
    QList<EpisodeInfo> Episodes;
};


struct TVCollection {
    QList<ShowInfo> Show;
};

class MediaLibrary : public QObject
{
    Q_OBJECT
public:
    MediaLibrary(QObject *parent, DBManager *dbmanager, SettingsData *settingsData);
    ~MediaLibrary();
    QStringList scanLibraryMovie(QString path);
    void refsreshCollectionMovie();
    void refsreshCollectionTV();
    movieCollections getMovieCollection(QString detailLevel);
    TVCollection getTVCollection(QString detailLevel);
    void sendRequestTMDBSearch(QString nameMedia, QString type="multi");
public slots:
    void slotFinishRequest(QNetworkReply *reply);
private:
    void handleProgressUpdate(QString str);
    void handleProgressFinish(QStringList str, QString type);
    void startScanLibraryMovie();
    void startScanLibraryTV();
    void removeOldMoviesInDB();
    void removeOldTVInDB();
    SettingsData *m_settingsData;
    DBManager *m_dbmanager;
    WorkRefreshMovie *m_workRMovie;
    QThread *m_workerThread;
    int progress;
    void sortEpisodes(QList<EpisodeInfo> &episodes);
    void sortShows(QList<ShowInfo> &shows);
signals:
    // void updateProgressBarUI(int i);
    void updateProgressBarUI(QString str);
};

#endif // MEDIALIBRARY_H
