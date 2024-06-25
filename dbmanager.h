#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QStandardPaths>

class DBManager
{
private:
    QSqlDatabase m_database;
    const QString DB_NAME = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/MediaFinder/mediafinder.sqlite";

public:
    DBManager();
    ~DBManager();
    bool createTables();
    bool writeLibrary(QString path, QString type);
    bool truncateTable(QString tbName);
    QStringList readLibrary();
    QStringList readLibrary(QString type);
    bool checkConnectingDB();
private:
    bool openConnection();
    void closeConnection();
    void checkDB();
    void createStructureDB(QString nameTables);

};

#endif // DBMANAGER_H
