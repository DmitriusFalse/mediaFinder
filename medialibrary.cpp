#include "medialibrary.h"
#include "settingsdata.h"
#include "workrefreshmovie.h"
#include <QDir>
#include <QIcon>
#include <QFile>

MediaLibrary::MediaLibrary(QObject *parent, DBManager *dbmanager, SettingsData *settingsData)
    : QObject(parent)
    , m_settingsData(settingsData)
    , m_dbmanager(dbmanager)
{
    this->progress=0;

    this->m_workerThread = new QThread(this);
    this->m_workRMovie = new WorkRefreshMovie();

    connect(m_workRMovie, &WorkRefreshMovie::progressUpdated, this, &MediaLibrary::handleProgressUpdate);
    connect(m_workRMovie, &WorkRefreshMovie::taskScanFolderFinished, this, &MediaLibrary::handleProgressFinish);
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

movieCollections MediaLibrary::getMovieInBase(QString detailLevel)
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



void MediaLibrary::handleProgressUpdate(QString str)
{
    emit updateProgressBarUI(str);
}

void MediaLibrary::handleProgressFinish(QStringList str)
{
    m_dbmanager->writeMovieCollectionToDB(str);
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

void MediaLibrary::removeOldMoviesInDB()
{
    this->m_dbmanager->removeOldRecordInBD ("Movie");
}
