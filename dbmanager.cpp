#include "dbmanager.h"
#include "qfileinfo.h"
#include <QSqlError>
#include <QMessageBox>
#include <QApplication>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>

DBManager::DBManager(QObject *parent)
    : QObject(parent)
{
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
        this->m_database.removeDatabase (DB_NAME);

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
    query.finish();
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
        query.finish();
        return false;
    }
    query.finish();
    return true;
}

bool DBManager::truncateTable(QString tbName)
{
    QSqlQuery query(this->m_database);
    query.prepare("DELETE FROM "+tbName);
    if(!query.exec()){
        qDebug() << "DELETE" << tbName << " DB false: " << query.lastError().text();
        query.finish();
        return false;
    }
    query.finish();
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
    query.finish();
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
    query.finish();
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

void DBManager::writeMovieCollectionToDB(QStringList pathlList)
{


    emit signalUpdateProgresBar ("Обновляем Базу Данных");
    QStringList srcData;
    QStringList libraryPaths;
    srcData.append(DBManager::readLibrary("Movie"));

    for (auto& str : srcData) {
        QStringList split = str.split(":");
        libraryPaths << split[0];
    }

    for (auto& item : pathlList) {

        QFileInfo info(item);
        //Проверяем наличе в БД, если есть пропуск, если то проверяем на наличие файлов и если на месте, то пропускаем, если нету то удаляем
        QSqlQuery query(this->m_database);
        query.prepare("SELECT * FROM Movie WHERE path = :path");
        query.bindValue(":path", item);

        if (query.exec()) {
            if (query.next()) {
                continue;
            }
        }
        bool okInsert = false;
        QString libraryPath;
        for (auto& p : libraryPaths) {
            if(item.startsWith(p) && (item == p || item.mid(p.length(), 1) == "/")) {
                libraryPath = p;
                okInsert = true;
                break;
            }
        }
        if(!okInsert){
            continue;
        }
        QString Genre = "";
        QString path = info.canonicalFilePath();
        QFileInfo posterFile(info.absolutePath()+"/poster.png");
        QString poster;
        if(posterFile.exists()){
            poster = posterFile.canonicalFilePath();
        }else{
            poster = "/opt/MediaFinder/poster.png";
        }
        QString name = info.completeBaseName();
        QString description = "";

        query.clear();
        query.prepare("INSERT INTO Movie (Genre,path,poster,name,Library_path,Description) VALUES "
                      "(:genre,:path,:poster,:name,:library,:descripton)");
        query.bindValue(":genre", Genre);
        query.bindValue(":path", path);
        query.bindValue(":poster", poster);
        query.bindValue(":name", name);
        query.bindValue(":library", libraryPath);
        query.bindValue(":descripton", description);
        if(!query.exec()){
            qDebug() << query.lastError ().text ();
            qDebug() << "name " << name;
            qDebug() << "libraryPath " << libraryPath;
        }

        query.finish();
    }
    emit signalUpdateMainWindow ("Movie");
    // INSERT INTO Movie (id,Genre,"path",poster,name,Library_path,Description) VALUES
    //     (1,'Kino','/home/noc101','/home/noc101/poster.png','Life','/home/noc101','My shit life');
}

void DBManager::writeTVCollectionToDB(QStringList pathlList)
{

}

void DBManager::removeMovieCollectionsLibrary(QString libraryPath)
{
    QSqlQuery query(this->m_database);
    query.prepare("DELETE FROM Movie WHERE Library_path=:path");
    query.bindValue(":path", libraryPath);
    query.exec ();
    query.finish ();
}

void DBManager::removeOldRecordInBD(QString type)
{
    const QString MOVIE = "Movie";
    const QString TV = "TV";
    emit signalUpdateProgresBar ("Чистим базу от старых записей!");
    if (type == MOVIE){
        //Получаем список фильмов из БД которые записаны в базу
        QSqlQuery query(this->m_database);
        query.prepare("SELECT id, path FROM Movie");
        if(query.exec ()){
            while(query.next ()){
                QString path = query.value ("path").toString ();
                QFileInfo file(path);
                if(!file.exists ()){
                    int id = query.value ("id").toInt ();
                    QSqlQuery subQuery(this->m_database);
                    subQuery.prepare("DELETE FROM Movie WHERE id=:id");
                    subQuery.bindValue (":id", id);
                    if(!subQuery.exec()){
                        qDebug() << "Error: " << subQuery.lastError ().text ();
                    }
                }
            }
        }
    }else if (type==TV) {

    }

}

QStringList DBManager::readMovieCollection(QString detailLevel)
{
    // qDebug() << "readMovieCollection";
    const QString DETAIL_LEVEL_ALL = "all";
    const QString DETAIL_LEVEL_SHORT = "short";
    QSqlQuery query(this->m_database);
    QStringList listMovies={};
    query.prepare("SELECT * FROM Movie");
    if(query.exec ()){
        while (query.next()) {
            QString id = query.value("id").toString();
            QString poster = query.value("poster").toString();
            QString name = query.value("name").toString();

            if (detailLevel == DETAIL_LEVEL_ALL) {
                QString genre = query.value("Genre").toString();
                QString path = query.value("path").toString();
                QString libraryPath = query.value("Library_path").toString();
                QString description = query.value("Description").toString();

                QString formattedEntry = QString("%1//%2//%3//%4//%5//%6//%7")
                                             .arg(id, genre, path, poster, name, libraryPath, description);

                listMovies.append(formattedEntry);
            } else if (detailLevel == DETAIL_LEVEL_SHORT) {

                QString formattedEntry = QString("%1//%2//%3")
                                             .arg(id, poster, name);

                listMovies.append(formattedEntry);
            }
        }
    }

    return listMovies;
}







