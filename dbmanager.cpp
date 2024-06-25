#include "dbmanager.h"
#include <QSqlError>
#include <QMessageBox>
#include <QApplication>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>

DBManager::DBManager() {
    this->m_database = QSqlDatabase::addDatabase("QSQLITE", "MediaFinder");
    this->m_database.setDatabaseName(DB_NAME);

    if (this->openConnection()) {
        this->checkDB();
    } else {
        QMessageBox::critical(QApplication::activeWindow(), "Ошибка", "Ошибка открытия базы данных!", QMessageBox::Ok);
    }
}

DBManager::~DBManager() {
    this->closeConnection();
}

void DBManager::closeConnection() {
    if (this->m_database.isOpen()) {
        this->m_database.close();
        QSqlDatabase::removeDatabase("MediaFinder");
    }
}

bool DBManager::openConnection() {
    if (!this->m_database.open()) {
        qDebug() << "Ошибка открытия соединения с базой данных:"
                 << this->m_database.lastError().text();
        return false;
    }
    return true;
}

void DBManager::checkDB() {
    QStringList tables = this->m_database.tables();
    QStringList tbDB = {"Movie", "Library"};

    for (auto& table : tbDB) {
        if (!tables.contains(table)) {
            this->createStructureDB(table);
        }
    }
}

void DBManager::createStructureDB(QString nameTables) {
    QStringList actionTb = {"Movie", "Library"};

    QSqlQuery query(this->m_database);

    switch (actionTb.indexOf(nameTables)) {
    case 0:
        query.exec("CREATE TABLE Movie (id INTEGER PRIMARY KEY, type TEXT, path TEXT UNIQUE, poster TEXT, name TEXT)");
        break;
    case 1:
        query.exec("CREATE TABLE Library (id INTEGER PRIMARY KEY, path TEXT UNIQUE, type TEXT)");
        break;
    }
}

bool DBManager::writeLibrary(QString path, QString type) {
    QSqlQuery query(this->m_database);

    query.prepare("SELECT * FROM Library WHERE path = :path");
    query.bindValue(":path", path);

    if (query.exec()) {
        if (query.next()) {
            int id = query.value("id").toInt();
            query.prepare("UPDATE Library SET path = :path, type = :type WHERE id = :id");
            query.bindValue(":path", path);
            query.bindValue(":type", type);
            query.bindValue(":id", id);
        } else {
            query.prepare("INSERT INTO Library (path, type) VALUES (:path, :type)");
            query.bindValue(":path", path);
            query.bindValue(":type", type);
            query.exec(); // Запускаем запрос INSERT
        }
    } else {
        return false;
    }

    return true;
}

bool DBManager::truncateTable(QString tbName)
{
    QSqlQuery query(this->m_database);
    query.prepare("DELETE FROM "+tbName);
    if(!query.exec()){
        qDebug() << "DELETE" << tbName << " DB false: " << query.lastError().text();
        return false;
    }
    return true;
}

QStringList DBManager::readLibrary()
{

    QStringList data;
    QSqlQuery query(this->m_database);

    // Подготавливаем запрос только если соединение с базой данных открыто
    if (m_database.isOpen()) {
        query.prepare("SELECT * FROM Library");

        if (query.exec()) {
            while (query.next()) {
                QString path = query.value("path").toString();
                QString type = query.value("type").toString();
                data.append(path + ":" + type);
            }
        } else {
            qDebug() << "Error executing query:" << query.lastError().text();
        }
    }
    return data;
}

QStringList DBManager::readLibrary(QString type)
{
    QStringList data;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT path, type FROM Library WHERE type = :type");
    query.bindValue(":type", type);

    if (query.exec()) {
        while(query.next()){
            data.append(query.value("path").toString() + ":" + query.value("type").toString());
        }
    }
    return data;
}

bool DBManager::checkConnectingDB()
{
    if (!this->m_database.open()) {
        qDebug() << "Ошибка открытия соединения с базой данных:"
                 << this->m_database.lastError().text();
        return false;
    }

    return true;
}

