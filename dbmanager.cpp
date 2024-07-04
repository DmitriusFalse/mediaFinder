#include "dbmanager.h"
#include "qdir.h"
#include "qfileinfo.h"
#include "qregularexpression.h"
#include <QSqlError>
#include <QMessageBox>
#include <QApplication>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
QRegularExpression DBManager::re(".*[sS](\\d+)[eE](\\d+).*");
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

    emit signalUpdateProgresBar ("Обновляем Базу Данных");
    //Сортируем на всякий пожарный
    pathlList.sort ();
    //Получаем список наших библиотек, для получения пути к онной
    QStringList srcData;
    QStringList libraryPaths;
    srcData.append(DBManager::readLibrary("TV"));

    for (const auto& str : srcData) {
        QStringList split = str.split(":");
        libraryPaths << split[0];
    }

    QSqlQuery query(this->m_database);

    foreach (auto& fullPath, pathlList) {
        //Информация о конечном файле
        QFileInfo fileInfo(fullPath);
        //Полный путь к конечному файлу
        QString File = fileInfo.filePath ();
        QString NameEpisode = fileInfo.baseName ();
        //Ищем в базе такой же файл.

        query.prepare("SELECT id FROM TVEpisodes WHERE File = :path");
        query.bindValue(":path", File);
        if (query.exec() && query.next()) {
            // Если он есть, то мы пропускаем файл
            continue;
        }

        bool okInsert = false;
        QString LibraryPath;
        for (auto& p : libraryPaths) {
            if(fullPath.startsWith(p) && (fullPath == p || fullPath.mid(p.length(), 1) == "/")) {
                LibraryPath = p;
                okInsert = true;
                break;
            }
        }
        if(!okInsert){
            continue;
        }
        //Узнаем имя Шоу
        // Если конечный файл лежит в папке Season ## то поднимаемся выше и берем название сериала из папки выше
        // Иначе берем название из текущей папки
        // Заодно пишем название путь к сериалу
        QString NameShow = "";
        QString PathToSerial = "";
        if(fileInfo.dir ().dirName().startsWith ("season",Qt::CaseInsensitive)){
            QDir dir = fileInfo.dir();
            //Поднимаемся на уровень выше
            dir.cdUp ();
            NameShow = dir.dirName ();
            PathToSerial = dir.canonicalPath ();
        }else{
            QDir dir = fileInfo.dir();
            NameShow = fileInfo.dir ().dirName();
            PathToSerial = dir.canonicalPath ();
        }
        // Если постер есть, то добавляем его в БД
        QString poster = "";
        if(QFile::exists (PathToSerial+"/poster.png")){
            poster = PathToSerial+"/poster.png";
        }else{
            // Или пишем стандартный постер
            poster = "/opt/MediaFinder/poster.png";
        }
        QString NumSeason = "0";
        QString NumEpisode = "0";

        QRegularExpressionMatch match = DBManager::re.match(fileInfo.fileName ());
        if (match.hasMatch()) {
            NumSeason = match.captured(1);  // Извлекаем номер сезона
            NumEpisode = match.captured(2); // Извлекаем номер серии
        }

        query.prepare("SELECT id FROM TVShow WHERE NameShow = :name");
        query.bindValue(":name", NameShow);
        if(query.exec ()){
            if(!query.next ()){
                query.clear ();
                query.prepare ("INSERT INTO TVShow (NameShow, Description, Poster) VALUES (:name, :desc, :poster)");
                query.bindValue (":name", NameShow);
                query.bindValue (":desc", "");
                query.bindValue (":poster", poster);
                if (!query.exec()) {
                    qDebug() << query.lastError ().text ();
                }
            }
        }
        // INSERT INTO TVShow (NameShow, Description, Poster) VALUES (:name, :desc, :poster);

        query.clear ();

        query.prepare("SELECT id FROM TVEpisodes WHERE File = :File");
        query.bindValue(":File", File);

        if (query.exec()) {
            if(query.next()){
                continue;
            }
        }

        // INSERT INTO TVEpisodes (NameShow, PathToSerial, LibraryPath, Description, Poster, File, Episode, Season )
        //                 VALUES (:name,      :path,        :library,    :desc,       :poster,:file,:ep,     :seas);

        query.prepare ("INSERT INTO TVEpisodes (NameShow, PathToSerial, LibraryPath, Description, Poster, File, Episode, Season, NameEpisode ) "
                      "VALUES (:name,      :path,        :library,    :desc,       :poster,:file,:ep,     :seas, :NameEpisode)");
        query.bindValue (":name", NameShow);
        query.bindValue (":path", PathToSerial);
        query.bindValue (":library", LibraryPath);
        query.bindValue (":desc", "");
        query.bindValue (":poster", "/opt/MediaFinder/poster.png");
        query.bindValue (":file", File);
        query.bindValue (":ep", NumEpisode);
        query.bindValue (":seas", NumSeason);
        query.bindValue (":NameEpisode", NameEpisode);

        if (!query.exec()) {
            qDebug() << query.lastError ().text ();
        }
        query.clear ();

    }
    emit signalUpdateMainWindow ("TV");

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
    QSqlQuery query(this->m_database);
    if (type == MOVIE){
        //Получаем список фильмов из БД которые записаны в базу

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
        query.prepare("SELECT ID, NameShow, File FROM TVEpisodes");
        if(query.exec ()){
            while(query.next ()){
                if(QFile::exists (query.value ("File").toString ())){
                    int id = query.value ("ID").toInt ();
                    QSqlQuery subQuery(this->m_database);
                    subQuery.prepare("DELETE FROM TVEpisodes WHERE ID=:id");
                    subQuery.bindValue (":id", id);
                    if(!subQuery.exec()){
                        qDebug() << "Error: " << subQuery.lastError ().text ();
                    }else{
                        QString nameShow = query.value ("NameShow").toString ();
                        subQuery.clear ();
                        subQuery.prepare("SELECT COUNT(*) FROM TVEpisodes WHERE nameShow = :name");
                        subQuery.bindValue(":name", nameShow); // Безопаснее использовать привязки
                        if(subQuery.exec()){
                            if(subQuery.next ()){
                                int episodeCount = subQuery.value(0).toInt();
                                if(episodeCount == 0 ){
                                    subQuery.clear ();
                                    subQuery.prepare ("DELETE FROM TVSHow WHERE NameShow = :show");
                                    subQuery.bindValue (":show", nameShow);
                                    subQuery.exec ();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

QStringList DBManager::loadGenre()
{
    QStringList genres;
    QSqlQuery query(this->m_database);
    query.prepare ("SELECT id, ru FROM Genres");
    if(query.exec ()){
        while(query.next()){
            genres.append (query.value ("id").toString ()+":"+query.value ("ru").toString ());
        }
    }
    return genres;
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

QStringList DBManager::readTVCollection(QString detailLevel)
{
    const QString DETAIL_LEVEL_ALL = "all";
    const QString DETAIL_LEVEL_SHORT = "short";

    QStringList listTV={};
    //Получаем список сериалов в Базе
    QSqlQuery queryTVShow(this->m_database);
    queryTVShow.prepare("SELECT id, NameShow, Poster FROM TVShow");
    if(queryTVShow.exec ()){
        //Обрабатываем каждый сериал отдельно
        while (queryTVShow.next()) {

            QString Show = ""; // Строка с основной информацией о сериале в целом

            QString IDShow = queryTVShow.value ("ID").toString ();
            QString NameShow = queryTVShow.value ("NameShow").toString ();
            QString PosterShow = queryTVShow.value ("Poster").toString ();
            Show = QString("%1//%2//%3")
                               .arg(IDShow, NameShow, PosterShow);
            Show = Show + "//@//"; //header info о сериале //@// - разделитель между информации о сериале и информации о сериях
            // Получаем информацию о сериях сериала!
            QSqlQuery querySeries(this->m_database);
            querySeries.prepare ("SELECT * FROM TVEpisodes WHERE NameShow = :nameShow");
            querySeries.bindValue (":nameShow", NameShow);

            QString body=""; // Строка с основной информацией о сериях сериала
            if(querySeries.exec ()){
                while (querySeries.next ()){

                    QString IDEpisode = querySeries.value("ID").toString();

                    QString PathToSerial = querySeries.value("PathToSerial").toString();
                    QString Poster = querySeries.value("Poster").toString();
                    QString Episode = querySeries.value("Episode").toString();
                    QString Season = querySeries.value("Season").toString();
                    QString File = querySeries.value("File").toString();
                    QString NameEpisode = querySeries.value("NameEpisode").toString();

                    if (detailLevel == DETAIL_LEVEL_ALL) { // Полная информация

                        QString LibraryPath = querySeries.value("LibraryPath").toString();
                        QString Description = querySeries.value("Description").toString();

                        body = QString("%1//%2//%3//%4//%5//%6//%7//%8//%9")
                        .arg(IDEpisode,NameEpisode,PathToSerial, LibraryPath, Description, Poster, File, Season, Episode);
                    } else if (detailLevel == DETAIL_LEVEL_SHORT) { // Короткая информация

                        body = QString("%1//%2//%3//%4//%5//%6")
                        .arg(IDEpisode, NameEpisode, File, Poster, Season, Episode);
                    }
                    Show = Show + "#/@/#" + body;
                }
                listTV.append (Show);
            }else{
                qDebug() << querySeries.lastError ().text ();
            }


        }
    }else{
        qDebug() << queryTVShow.lastError ().text ();
    }
    return listTV;
}







