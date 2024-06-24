#include "dbmanager.h"
#include <QCoreApplication>
DBManager::DBManager() {
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << DB_NAME;
    m_database.setDatabaseName(QStringLiteral(DB_NAME));
}

bool DBManager::openConnection()
{

}
