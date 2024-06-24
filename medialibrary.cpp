#include "medialibrary.h"
#include "settingsdata.h"
#include <QDir>
#include <QIcon>
#include <QFile>

MediaLibrary::MediaLibrary() {

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
    QStringList fileExt = m_settingsData.getVideoExtensions();
    foreach(const QString &fileName, scanList) {
        QFileInfo fileInfo(path+"/"+fileName);
        if(fileInfo.isFile()) {
            QString fileSuffix = fileInfo.suffix();
            if (fileExt.contains(fileSuffix)) {
                listMovie.append(path+"/"+fileName);
            }
        } else if(fileInfo.isDir()) {
            listMovie.append(MediaLibrary::scanLibraryMovie(path+"/"+fileName));
        }
    }
    // delete settingsData;
    return listMovie;
}

movieCollections MediaLibrary::getRefsreshCollectionMovie()
{
    // SettingsData* settingsData = new SettingsData();
    QList<movieItem> itemList;
    movieCollections movieColl(itemList);

    QList<libraryItem> library = m_settingsData.readLibraryFromSettings("Movie");
    for (auto& libItem : library) {
        QStringList listMovie = MediaLibrary::scanLibraryMovie(libItem.path);

        for (auto& movie : listMovie ) {
            movieItem item;
            qDebug() << "Movie: " << movie;
            QFileInfo fileInfo(movie);
            item.fileDir = fileInfo.path();
            item.fileName = fileInfo.fileName();
            item.name = fileInfo.fileName();
            if (QFile::exists(item.fileDir+"/poster.png")) {
                item.pathIcon = item.fileDir+"/poster.png";
            } else {
                QString installPath = m_settingsData.getInstallPath();
                item.pathIcon = installPath+"/noposter.png";
            }
            movieColl.append(item);
        }
    }
    // delete settingsData;
    return movieColl;
}
