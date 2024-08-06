#ifndef DBMANAGER_H
#define DBMANAGER_H
#include "GenreList.h"
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
    GenreList loadGenre(QString lang);
    void saveGenres(GenreList genres);
    QList<Videos> getVideos(int idShow);
    QList<Reviews> getReviews(int idShow);
    TVCollection getTVCollection();
    MovieCollections getMovieCollection();
    QString readTVShowByID(QString detailLevel, int id);
    ShowInfo getShowTVShowByID(int id);
    MovieInfo getMovieByID(int id);
    QStringList readLibrary();
    QStringList readLibrary(QString type);
    void extracted(ShowInfo &show);
    void updateTvShow(ShowInfo show, int id);
    void updateMovie(MovieInfo movie, int id);
    void saveSettings(QString name, QVariant value);
    void putVault(QString name, QString value);
    QHash<QString,QString> getVault();
    QVariant getSetting(QString name);
    QMap<QString, QVariant> getAllSettings();
    QString getPathToShowTV(QString nameShow);
    QString getPathToMovie(QString name);
    QList<int> getListNumberSeason(int idDBTVShow);
    bool checkSeasonEpisodeExist(const QString NameShow, const int& season, const int& episode);
    bool updateMovieColumn(const QString &columnName, const QVariant &newValue, int rowId);
    void updateShowTVColumn(const QString &columnName, const QVariant &newValue, int rowId);
    bool updateEpisodeColumn(const QString &columnName, const QVariant &newValue, int rowId);
    void updateActorByShowTV(const QString &oldRootPath,const QString &newRootPath, const uint &idShowTV);
private:
    bool openConnection();
    void closeConnection();
    void checkDB();
    void createStructureDB(int index);
    void updateReviewsTV(QList<Reviews> reviews, int id);
    void updateTvShowEpisode(ShowInfo show);
    void updateVideosTV(QList<Videos> videos, QString nameShow, int id);
    // void createUpdateCrewEpisode(int idShow,int id,QString role, QString name,QString thumb);
    void createUpdateCrewEpisode(ShowInfo tvshow);
    void createUpdateCrewTVShow(ShowInfo tvshow);
    void createUpdateCrewMovie(MovieInfo movie);
    QString removeLeadingZeros(const QString& input);
    static QRegularExpression re;
signals:
    void signalUpdateProgresBar(QString value);
    void signalUpdateMainWindow(QString type);
    void signalUpdateMainWindowByID(QString type, int id);
    void signalUpdatePoster(QString url, QString NameShow);
};

#endif // DBMANAGER_H
