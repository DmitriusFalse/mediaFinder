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

QStringList WorkRefreshMovie::scanFoldersLibrary(QString path)
{
    QStringList fileExt = {"mp4", "avi", "mkv", "mov", "wmv", "flv", "mpeg", "mpg"};
    QStringList list={};
    QDir directory(path);
    if (!directory.exists()) {
        return list;
    }
    QStringList scanList = directory.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);

    foreach(const QString &fileName, scanList) {
        QFileInfo fileInfo(path+"/"+fileName);
        if(fileInfo.isFile()) {
            QString fileSuffix = fileInfo.suffix();
            if (fileExt.contains(fileSuffix)) {
                emit progressUpdated(tr("Нашли: ")+fileName);
                list.append(path+"/"+fileName);
            }
        } else if(fileInfo.isDir()) {
            list.append(this->scanFoldersLibrary(path+"/"+fileName));
        }
    }
    return list;
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


void WorkRefreshMovie::run()
{
    QStringList list={};
    for (auto& path : this->pathList) {
        QStringList listPath = scanFoldersLibrary(path);
        list.append(listPath);
    }
    switch (this->action) {
    case 0:{
        emit signalFinishScanFoldersLibrary(list, "movie");
        break;
    }
    case 1:{
        emit signalFinishScanFoldersLibrary(list, "tv");
        break;
    }
    }


}
