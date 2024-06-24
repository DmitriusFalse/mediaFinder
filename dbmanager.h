#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QSqlDatabase>
#include <QSqlQuery>

class DBManager
{
private:
    QSqlDatabase m_database;
public:
    DBManager();
    bool openConnection();
    void closeConnection();
    bool createTables();
};

#endif // DBMANAGER_H
