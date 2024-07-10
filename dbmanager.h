#ifndef DBMANAGER_H
#define DBMANAGER_H
#include "movieCollections.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QStandardPaths>
#include <QObject>

class DBManager : public QObject
{
    Q_OBJECT
private:
    QSqlDatabase m_database;
    const QString DB_NAME = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/MediaFinder/mediafinder.sqlite";

public:
    DBManager(QObject *parent);
    ~DBManager();
    bool createTables();
    bool writeLibrary(QString path, QString type);
    bool truncateTable(QString tbName);
    bool checkConnectingDB();
    void writeMovieCollectionToDB(QStringList pathlList);
    void writeTVCollectionToDB(QStringList pathlList);
    void removeMovieCollectionsLibrary(QString libraryPath);
    void removeOldRecordInBD(QString type);
    QStringList loadGenre();
    QStringList readMovieCollection(QString detailLevel);
    QStringList readTVCollection(QString detailLevel);
    QString readTVShowByID(QString detailLevel, int id);
    ShowInfo getShowTVShowByID(int id);
    QStringList readLibrary();
    QStringList readLibrary(QString type);
    void extracted(ShowInfo &show);
    void updateTvShow(ShowInfo show, int id);
    void updateTvShowPoster(QString url, QString NameShow);
    QString getPathToShowTV(QString nameShow);
private:
    bool openConnection();
    void closeConnection();
    void checkDB();
    void createStructureDB(QString nameTables);
    void updateReviewsTV(QList<Reviews> reviews, QString NameShow);
    void updateTvShowEpisode(ShowInfo show);
    void updateVideosTV(QList<Videos> videos, QString NameShow);
    static QRegularExpression re;
signals:
    void signalUpdateProgresBar(QString value);
    void signalUpdateMainWindow(QString type);
    void signalUpdateMainWindowByID(QString type, int id);
    void signalUpdatePoster(QString url, QString NameShow);
};

#endif // DBMANAGER_H
