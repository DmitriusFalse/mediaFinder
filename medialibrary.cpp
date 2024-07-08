#include "medialibrary.h"
#include "settingsdata.h"
#include "workrefreshmovie.h"
#include <QDir>
#include <QIcon>
#include <QFile>
#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

MediaLibrary::MediaLibrary(QObject *parent, DBManager *dbmanager, SettingsData *settingsData)
    : QObject(parent)
    , m_settingsData(settingsData)
    , m_dbmanager(dbmanager)
{
    this->progress=0;

    this->m_workerThread = new QThread(this);
    this->m_workRMovie = new WorkRefreshMovie();

    connect(m_workRMovie, &WorkRefreshMovie::progressUpdated, this, &MediaLibrary::handleProgressUpdate);
    connect(m_workRMovie, &WorkRefreshMovie::signalFinishScanFoldersLibrary, this, &MediaLibrary::handleProgressFinish);
}

MediaLibrary::~MediaLibrary()
{
    m_workerThread->quit();
    m_workerThread->wait();
    delete m_workerThread;
}

QStringList MediaLibrary::scanLibraryMovie(QString path)
{

    // SettingsData* settingsData = new SettingsData();
    QDir directory(path);
    if (!directory.exists()) {
        qWarning() << "Directory does not exist.";
    }
    QStringList listMovie;
    QStringList scanList = directory.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList fileExt = m_settingsData->getVideoExtensions();
    foreach(const QString &fileName, scanList) {
        QFileInfo fileInfo(path+"/"+fileName);
        if(fileInfo.isFile()) {
            QString fileSuffix = fileInfo.suffix();
            if (fileExt.contains(fileSuffix)) {
                emit m_workRMovie->progressUpdated(path+"/"+fileName);

                listMovie.append(path+"/"+fileName);
            }
        } else if(fileInfo.isDir()) {
            listMovie.append(MediaLibrary::scanLibraryMovie(path+"/"+fileName));
        }
    }

    return listMovie;
}

void MediaLibrary::refsreshCollectionMovie()
{
    this->removeOldMoviesInDB ();
    MediaLibrary::startScanLibraryMovie();
}

void MediaLibrary::refsreshCollectionTV()
{
    this->removeOldTVInDB();
    this->startScanLibraryTV ();
}

movieCollections MediaLibrary::getMovieCollection(QString detailLevel)
{
    const QString DETAIL_LEVEL_ALL = "all";
    const QString DETAIL_LEVEL_SHORT = "short";



    QList<movieItem> items={};
    movieCollections movies(items);
    QStringList srcMovies = this->m_dbmanager->readMovieCollection (detailLevel);
    //
        //

    for (auto& item : srcMovies) {
        movieItem srcMovieItem;
        QStringList iMovie = item.split ("//");
        if (detailLevel == DETAIL_LEVEL_ALL) {
            // id, genre, path, poster, name, libraryPath, description
            // 0   1      2     3       4     5            6
            srcMovieItem.id = iMovie[0].toInt ();
            srcMovieItem.genre = iMovie[1];
            srcMovieItem.path = iMovie[2];
            srcMovieItem.poster = iMovie[3];
            srcMovieItem.name = iMovie[4];
            srcMovieItem.library_path = iMovie[5];
            srcMovieItem.description = iMovie[6];
        } else if (detailLevel == DETAIL_LEVEL_SHORT) {
            // id, poster, name
            // 0   1       2
            srcMovieItem.id = iMovie[0].toInt ();
            srcMovieItem.poster = iMovie[1];
            srcMovieItem.name  = iMovie[2];
        }
        movies.append (srcMovieItem);

    }
    return movies;
}

TVCollection MediaLibrary::getTVCollection(QString detailLevel)
{
    // const QString DETAIL_LEVEL_ALL = "all";
    // const QString DETAIL_LEVEL_SHORT = "short";
    TVCollection tvcol;
    QStringList srcTV = this->m_dbmanager->readTVCollection (detailLevel);

    for (const QString& SrcShow : srcTV) {
        ShowInfo show;

        QStringList tmp= SrcShow.split ("//@//");
        QString header = tmp[0];
        QStringList srcInfoShow = header.split ("//");
        show.ID = srcInfoShow[0].toInt ();
        show.nameShow = srcInfoShow[1];
        show.poster = srcInfoShow[2];

        QString body= tmp[1];
        QStringList srcSeries = body.split ("#/@/#");

        foreach (const QString& infoEp, srcSeries) {
            if (infoEp==""){
                continue;
            }
            QStringList srcEpList = infoEp.split ("//");
            EpisodeInfo episode;
            episode.ID = srcEpList[0].toInt ();
            episode.episodeTitle = srcEpList[1];
            episode.filePath = srcEpList[2];
            episode.seasonsNumber = srcEpList[4].toInt ();
            episode.episodeNumber = srcEpList[5].toInt ();

            show.addEpisodes(episode);
        }
        //сортируем наши епизоды

        // this->sortEpisodes (show.Episodes);
        tvcol.Show.append (show);

    }
    this->sortShows (tvcol.Show);
    return tvcol;
}

ShowInfo MediaLibrary::getShowInfoByID(QString detailLevel, int id)
{

    ShowInfo show;
    QString SrcShow = this->m_dbmanager->readTVShowByID (detailLevel, id);

    QStringList tmp= SrcShow.split ("//@//");
    QString header = tmp[0];
    QStringList srcInfoShow = header.split ("//");
    show.ID = srcInfoShow[0].toInt ();
    show.nameShow = srcInfoShow[1];
    show.poster = srcInfoShow[2];

    QString body= tmp[1];
    QStringList srcSeries = body.split ("#/@/#");

    foreach (const QString& infoEp, srcSeries) {
        if (infoEp==""){
            continue;
        }
        QStringList srcEpList = infoEp.split ("//");
        EpisodeInfo episode;
        episode.ID = srcEpList[0].toInt ();
        episode.episodeTitle = srcEpList[1];
        episode.filePath = srcEpList[2];
        episode.seasonsNumber = srcEpList[4].toInt ();
        episode.episodeNumber = srcEpList[5].toInt ();

        show.addEpisodes(episode);
    }
    return show;
}

void MediaLibrary::handleProgressUpdate(QString str)
{
    emit updateProgressBarUI(str);
}
void MediaLibrary::handleProgressFinish(QStringList str, QString type)
{
    if (type=="movie"){
        m_dbmanager->writeMovieCollectionToDB(str);
    }else if(type=="tv"){
        m_dbmanager->writeTVCollectionToDB(str);
    }
}
void MediaLibrary::startScanLibraryMovie()
{

    QList<movieItem> itemList;
    movieCollections movieColl(itemList);

    QList<libraryItem> library = m_settingsData->readLibraryFromSettings("Movie");
    m_workRMovie->setActionScanMovie();
    m_workRMovie->clearPathList ();
    for (auto& libItem : library) {
        m_workRMovie->setPath(libItem.path);
    }
    m_workRMovie->start();
}
void MediaLibrary::startScanLibraryTV()
{
    TVCollection myTv;

    QList<libraryItem> library = m_settingsData->readLibraryFromSettings("TV");

    m_workRMovie->setActionScanTV ();
    m_workRMovie->clearPathList ();
    for (auto& libItem : library) {
        m_workRMovie->setPath(libItem.path);
    }
    m_workRMovie->start();

}
void MediaLibrary::removeOldMoviesInDB()
{
    this->m_dbmanager->removeOldRecordInBD ("Movie");
}
void MediaLibrary::removeOldTVInDB()
{
    qDebug() << "removeOldTVInDB";
    this->m_dbmanager->removeOldRecordInBD ("TV");
}

void MediaLibrary::sortEpisodes(QList<EpisodeInfo>& episodes) {
    std::sort(episodes.begin(), episodes.end(), [](const EpisodeInfo& a, const EpisodeInfo& b) {
        // Сравниваем по номеру сезона, если они равны - по номеру серии
        return a.seasonsNumber < b.seasonsNumber ||
               (a.seasonsNumber == b.seasonsNumber && a.episodeNumber < b.episodeNumber);
    });
}

void MediaLibrary::sortShows(QList<ShowInfo>& shows) {
    std::sort(shows.begin(), shows.end(), [](const ShowInfo& a, const ShowInfo& b) {
        // Сортировка по номеру сезона, затем по номеру серии
        return a.nameShow < b.nameShow;
    });
}
