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
        qWarning() << tr("Каталог не существует!");
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

    MovieCollections movieColl;

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
