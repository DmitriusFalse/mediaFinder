#include "workrefreshmovie.h"
#include "qdebug.h"
#include "qdir.h"

WorkRefreshMovie::WorkRefreshMovie(QObject *parent)
    : QThread(parent)
{

}
WorkRefreshMovie::~WorkRefreshMovie()
{

}

QStringList WorkRefreshMovie::scanFolders(QString path)
{
    QStringList fileExt = {"mp4", "avi", "mkv", "mov", "wmv", "flv", "mpeg", "mpg"};
    QStringList listMovie={};
    QDir directory(path);
    if (!directory.exists()) {
        return listMovie;
    }
    QStringList scanList = directory.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(const QString &fileName, scanList) {
        QFileInfo fileInfo(path+"/"+fileName);
        if(fileInfo.isFile()) {
            QString fileSuffix = fileInfo.suffix();
            if (fileExt.contains(fileSuffix)) {
                emit progressUpdated("Нашли: "+fileName);
                listMovie.append(path+"/"+fileName);
            }
        } else if(fileInfo.isDir()) {
            listMovie.append(this->scanFolders(path+"/"+fileName));
        }
    }

    return listMovie;
}
///
/// \brief WorkRefreshMovie::getAction
/// \return
///
int WorkRefreshMovie::getAction() const
{
    return action;
}
///
/// \brief WorkRefreshMovie::setActionScanMovie
///
void WorkRefreshMovie::setActionScanMovie()
{
    this->setAction(0);
}
///
/// \brief WorkRefreshMovie::setActionScanTV
///
void WorkRefreshMovie::setActionScanTV()
{
    this->setAction(1);
}
///
/// \brief WorkRefreshMovie::setActionRecordDBMovie
///
void WorkRefreshMovie::setActionRecordDBMovie()
{
    this->setAction(2);
}
///
/// \brief WorkRefreshMovie::setActionRecordDBTV
///
void WorkRefreshMovie::setActionRecordDBTV()
{
    this->setAction(3);
}
///
/// \brief WorkRefreshMovie::setAction
/// \param newAction
///
void WorkRefreshMovie::setAction(int newAction)
{
    action = newAction;
}

void WorkRefreshMovie::setPath(const QString &newPath)
{
    this->pathList << newPath;
}
void WorkRefreshMovie::clearPathList()
{
    this->pathList.clear();
}
void WorkRefreshMovie::startWork(const QString &path)
{
    // qDebug() <<"startWork is Work!";
    // QStringList listMovie = scanFolders(path);
    // emit taskScanFolderFinished(listMovie);
}


void WorkRefreshMovie::run()
{
    switch (this->action) {
    case 0:{ //Scan folder and find media Movie
        QStringList moviesList={};
        for (auto& path : this->pathList) {
            QStringList listMovie = scanFolders(path);
            moviesList.append(listMovie);
        }
        emit taskScanFolderFinished(moviesList);

        break;
    }
    case 1:{


        break;
    }
    }


}
