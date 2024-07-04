#ifndef DBMANAGER_H
#define DBMANAGER_H
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
    QStringList readLibrary();
    QStringList readLibrary(QString type);
private:
    bool openConnection();
    void closeConnection();
    void checkDB();
    void createStructureDB(QString nameTables);
    static QRegularExpression re;
signals:
    void signalUpdateProgresBar(QString value);
    void signalUpdateMainWindow(QString type);

};

#endif // DBMANAGER_H
