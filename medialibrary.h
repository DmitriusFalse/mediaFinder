#ifndef MEDIALIBRARY_H
#define MEDIALIBRARY_H
#include "movieCollections.h"
#include "settingsdata.h"
#include "workrefreshmovie.h"
#include <QObject>
#include <QString>


class MediaLibrary : public QObject
{
    Q_OBJECT
public:
    MediaLibrary(QObject *parent, DBManager *dbmanager, SettingsData *settingsData);
    ~MediaLibrary();
    QStringList scanLibraryMovie(QString path);
    void refsreshCollectionMovie();
    void refsreshCollectionTV();

public slots:

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
