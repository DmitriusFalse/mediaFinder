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
    QFileInfo file(DB_NAME);
    if(!QFile::exists(file.path())){
        QDir dir;
        dir.mkpath(file.path());
    }
    this->m_database = QSqlDatabase::addDatabase("QSQLITE", "MediaFinder");
    this->m_database.setDatabaseName(DB_NAME);
    if (this->openConnection()) {
        this->checkDB();
    } else {
        QMessageBox::critical(QApplication::activeWindow(), tr("Ошибка"), tr("Ошибка открытия базы данных!"), QMessageBox::Ok);
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
        qDebug() << tr("Ошибка открытия соединения с базой данных:")
                 << this->m_database.lastError().text();
        return false;
    }
    return true;
}

void DBManager::checkDB() {
    QStringList tables = this->m_database.tables();
    QStringList tbDB = {
        "Movie", "Library", "TVEpisodes", "Genres",
        "ReviewsTV", "TVShow", "VideosTV", "settings",
        "crewEpisode", "crewShowTV", "crewMovie", "Vault"
    };
    for (const QString& table : tbDB) {
        if (!tables.contains(table)) {
            qDebug() << tr("Создаем таблицу:") << tables;
            this->createStructureDB(tbDB.indexOf(table));
        }
    }
}

void DBManager::createStructureDB(int index) {

    QSqlQuery query(this->m_database);

    switch (index) {
    case 0:
        query.exec("CREATE TABLE IF NOT EXISTS Movie ("
                   "id INTEGER PRIMARY KEY, "
                   "Genre TEXT, "
                   "path TEXT UNIQUE, "
                   "poster TEXT, "
                   "name TEXT, "
                   "Library_path TEXT NOT NULL, "
                   "Description TEXT, "
                   "idMovie INTEGER, "
                   "originalLang TEXT, "
                   "imdbID INTEGER, "
                   "release_date TEXT, "
                   "originalName TEXT, "
                   "Status TEXT"
                   ")");
        break;
    case 1:
        query.exec("CREATE TABLE IF NOT EXISTS Library ("
                   "id INTEGER PRIMARY KEY, "
                   "path TEXT UNIQUE, "
                   "type TEXT"
                   ")");
        break;
    case 2:
        query.exec("CREATE TABLE IF NOT EXISTS TVEpisodes ("
                   "ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                   "NameShow TEXT NOT NULL, "
                   "PathToSerial TEXT NOT NULL, "
                   "LibraryPath TEXT NOT NULL, "
                   "Overview TEXT, "
                   "Poster TEXT, "
                   "File TEXT NOT NULL, "
                   "Episode TEXT, "
                   "Season TEXT, "
                   "NameEpisode TEXT, "
                   "idShow INTEGER, "
                   "air_date TEXT"
                   ")");
        break;
    case 3:
        query.exec("CREATE TABLE IF NOT EXISTS Genres ("
                   "id INTEGER NOT NULL, "
                   "ru TEXT, "
                   "en TEXT"
                   ")");
        break;
    case 4:
        query.exec("CREATE TABLE IF NOT EXISTS ReviewsTV ("
                   "nameShow TEXT NOT NULL, "
                   "author TEXT NOT NULL, "
                   "content TEXT NOT NULL, "
                   "idShow INTEGER"
                   ")");
        break;
    case 5:
        query.exec("CREATE TABLE IF NOT EXISTS TVShow ("
                   "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                   "NameShow TEXT NOT NULL, "
                   "Overview TEXT, "
                   "Poster TEXT, "
                   "Original_nameShow TEXT, "
                   "Number_of_episodes INTEGER, "
                   "Number_of_seasons INTEGER, "
                   "Status TEXT, "
                   "Genres TEXT, "
                   "IdShow INTEGER, "
                   "production_companies_name TEXT, "
                   "production_companies_logo_path TEXT, "
                   "first_air_date TEXT, "
                   "last_air_date TEXT, "
                   "imdb_id TEXT"
                   ")");
        break;
    case 6:
        query.exec("CREATE TABLE IF NOT EXISTS VideosTV ("
                   "NameShow TEXT, "
                   "key TEXT NOT NULL ON CONFLICT REPLACE, "
                   "idShow INTEGER"
                   ")");
        break;
    case 7:
        query.exec("CREATE TABLE IF NOT EXISTS settings ("
                   "name TEXT UNIQUE ON CONFLICT REPLACE, "
                   "value TEXT NOT NULL"
                   ")");
        break;
    case 8:
        query.exec("CREATE TABLE IF NOT EXISTS crewEpisode ("
                   "idShow INTEGER NOT NULL, "
                   "id INTEGER NOT NULL, "
                   "role TEXT, "
                   "name TEXT NOT NULL, "
                   "thumb TEXT, "
                   "CONSTRAINT crewEpisode_pk UNIQUE (idShow, id, role)"
                   ")");
        break;
    case 9:
        query.exec("CREATE TABLE IF NOT EXISTS crewShowTV ("
                   "idShow INTEGER NOT NULL, "
                   "id INTEGER, "
                   "name TEXT, "
                   "role TEXT, "
                   "thumb TEXT, "
                   "CONSTRAINT crewShowTV_pk UNIQUE (idShow, id, role)"
                   ")");
        break;
    case 10:
        query.exec("CREATE TABLE IF NOT EXISTS crewMovie ("
                   "idMovie INTEGER, "
                   "name TEXT, "
                   "role TEXT, "
                   "thumb TEXT, "
                   "id INTEGER, "
                   "CONSTRAINT crewMovie_pk UNIQUE (idMovie, name, role, id)"
                   ")");
        break;
    case 11:
        query.exec("CREATE TABLE vault ("
                   "name TEXT PRIMARY KEY ON CONFLICT REPLACE "
                   "UNIQUE ON CONFLICT REPLACE "
                   "NOT NULL, "
                   "data TEXT NOT NULL)");
        break;
    }
    query.finish();
}

void DBManager::updateReviewsTV(QList<Reviews> reviews, int id)
{
    QSqlQuery query(this->m_database);
    //Удаляем старые обзоры перед обновлением
    query.prepare("DELETE FROM ReviewsTV WHERE idShow = :id");
    query.bindValue(":id", id);
    query.exec();

    for (const auto& review : reviews) {
        query.clear();
        query.prepare("INSERT INTO ReviewsTV (nameShow, author, content, idShow) VALUES (:name, :a, :c, :id)");
        query.bindValue(":name", review.nameShow);
        query.bindValue(":a", review.author);
        query.bindValue(":c", review.content);
        query.bindValue(":id", review.idShow);
        if(!query.exec()){
            qDebug() << tr("Обновление обзоров - Ошибка выполнения запроса:") << query.lastError().text();
        }
    }
}

void DBManager::updateTvShowEpisode(ShowInfo show)
{
    QSqlQuery query(this->m_database);
    // Перебираем все сезоны и эпизоды сериала
    foreach (const uint seasonNumber, show.Episodes.keys()) {
        const QMap<uint, EpisodeInfo>& episodes = show.Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            const EpisodeInfo& episode = episodes[episodeNumber];

            // Обновляем информацию об эпизоде в таблице TVEpisodes
            query.prepare ("UPDATE TVEpisodes SET "
                          "Overview = :overview, "
                          "Poster = :poster, "
                          "NameEpisode = :nameEpisode, "
                          "idShow = :idShow, "
                          "air_date = :air_date "
                          "WHERE NameShow=:name AND Episode = :episode AND Season = :season");
            query.bindValue (":overview", episode.overview);
            query.bindValue (":poster", episode.still_path);
            query.bindValue (":nameEpisode", episode.episodeTitle);
            query.bindValue (":idShow", episode.ID);
            query.bindValue (":air_date", episode.air_date);
            query.bindValue (":name", show.nameShow);
            query.bindValue (":episode", episode.episodeNumber);
            query.bindValue (":season", episode.seasonsNumber);
            if (!query.exec()) {
                // Выводим сообщение об ошибке, если запрос не выполнен
                qDebug() << tr("Обновление серий - шибка выполнения запроса:") << query.lastError().text();
            }
        }
    }
}

void DBManager::updateVideosTV(QList<Videos> videos,QString nameShow, int id)
{
    QSqlQuery query(this->m_database);
    //Удаляем старые видео перед обновлением
    query.prepare("DELETE FROM VideosTV WHERE idShow = :id");
    query.bindValue(":id", id);
    query.exec();

    for (const Videos& video : videos) {
        query.prepare("INSERT INTO VideosTV (nameShow, key, idShow) VALUES (:nameShow, :key, :id)");
        query.bindValue(":nameShow",nameShow);
        query.bindValue(":key",video.key);
        query.bindValue(":id",video.idShow);
        if(!query.exec()){
            qDebug() << tr("Обновление видео из ютуба - шибка выполнения запроса:") << query.lastError().text();
        }
    }
}

void DBManager::createUpdateCrewEpisode(ShowInfo tvshow)
{
    QSqlQuery query(this->m_database);
    query.prepare("DELETE FROME crewEpisode WHERE idShow = :idShow");
    query.bindValue(":idShow", tvshow.idShow);
    query.exec();
    foreach (const uint seasonNumber, tvshow.Episodes.keys()) {
        QMap<uint, EpisodeInfo>& episodes = tvshow.Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            EpisodeInfo& episode = episodes[episodeNumber];
            while(episode.nextCrew()){
                Crew& crewData = episode.getCrew();
                //INSERT OR REPLACE INTO
                for (const QString& roleName : crewData.role) {
                    query.prepare("INSERT OR REPLACE INTO crewEpisode (idShow,id,role,name,thumb) VALUES (:idShow,:id,:role,:name,:thumb)");
                    query.bindValue(":idShow", tvshow.idShow);
                    query.bindValue(":id", crewData.id);
                    query.bindValue(":role",roleName);
                    query.bindValue(":name",crewData.name);
                    query.bindValue(":thumb",crewData.thumb);
                    if(!query.exec()){
                        qDebug() << tr("Обновление Команды серий - Ошибка выполнения запроса:") << query.lastError().text();
                    }
                }
            }
        }
    }
}

void DBManager::createUpdateCrewTVShow(ShowInfo tvshow)
{
    QSqlQuery query(this->m_database);
    query.prepare("DELETE FROME crewShowTV WHERE idShow = :idShow");
    query.bindValue(":idShow", tvshow.idShow);
    query.exec();
    while(tvshow.nextCrew()){
        Crew& crewData = tvshow.getCrew();
        //INSERT OR REPLACE INTO
        for (const QString& roleName : crewData.role) {
            query.prepare("INSERT OR REPLACE INTO crewShowTV (idShow,id,role,name,thumb) VALUES (:idShow,:id,:role,:name,:thumb)");
            query.bindValue(":idShow", tvshow.idShow);
            query.bindValue(":id", crewData.id);
            query.bindValue(":role",roleName);
            query.bindValue(":name",crewData.name);
            query.bindValue(":thumb",crewData.thumb);
            if(!query.exec()){
                qDebug() << tr("Обновление Команды шоу - Ошибка выполнения запроса:") << query.lastError().text();
            }
        }

    }
}

void DBManager::createUpdateCrewMovie(MovieInfo movie)
{
    QSqlQuery query(this->m_database);
    query.prepare("DELETE FROME crewMovie WHERE idMovie = :idMovie");
    query.bindValue(":idMovie", movie.IDMovie);
    query.exec();
    while(movie.nextCrew()){
        Crew& crewData = movie.getCrew();
        for (const QString& roleName : crewData.role) {
            query.prepare("INSERT OR REPLACE INTO crewMovie (idMovie,role,name,thumb, id) VALUES (:idMovie,:role,:name,:thumb, :id)");
            query.bindValue(":idMovie", movie.IDMovie);
            query.bindValue(":id",crewData.id);
            query.bindValue(":role",roleName);
            query.bindValue(":name",crewData.name);
            query.bindValue(":thumb",crewData.thumb);
            if(!query.exec()){
                qDebug() << tr("Обновление Команды Фильма - Ошибка выполнения запроса:") << query.lastError().text();
            }
        }

    }
}

QString DBManager::removeLeadingZeros(const QString &input)
{
    QString result = input;
    // Удаляем ведущие нули
    while (result.startsWith('0') && result.length() > 1) {
        result.remove(0, 1);
    }
    return result;

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
            qDebug() << tr("Ошибка выполнения запроса:") << query.lastError().text();
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

void DBManager::updateTvShow(ShowInfo showTV, int id)
{
    QSqlQuery query(this->m_database);
     // Получаем старое название сериала по ID
    query.prepare ("SELECT NameShow FROM TVShow WHERE id=:id");
    query.bindValue (":id", id);
    if(query.exec ()){
        query.next ();
        QString oldName = query.value ("NameShow").toString ();
        // Обновляем информацию о сериале в таблице TVShow
        query.prepare("UPDATE TVShow SET "
                      "NameShow=:nameShow,"
                      "Overview=:overview,"
                      "Poster=:poster,"
                      "Original_nameShow=:originalNameShow,"
                      "Number_of_episodes=:numberOfEpisodes,"
                      "Number_of_seasons=:numberOfSeasons,"
                      "Status=:status,"
                      "Genres=:genres,"
                      "IdShow=:idShow,"
                      "production_companies_name=:proName,"
                      "production_companies_logo_path=:logo,"
                      "first_air_date=:fdate,"
                      "last_air_date=:ldate,"
                      "imdb_id=:imdb "
                      "WHERE id=:id");
        query.bindValue (":nameShow",showTV.nameShow);
        query.bindValue (":overview",showTV.overview);
        query.bindValue (":poster",showTV.poster);
        query.bindValue (":originalNameShow",showTV.originalNameShow);
        query.bindValue (":numberOfEpisodes",showTV.numberOfEpisodes);
        query.bindValue (":numberOfSeasons",showTV.numberOfSeasons);
        query.bindValue (":status",showTV.status);
        query.bindValue (":genres",showTV.genres);
        query.bindValue (":idShow",showTV.idShow);
        query.bindValue (":proName",showTV.production_companies);
        query.bindValue (":logo",showTV.logoPath);
        query.bindValue (":fdate",showTV.first_air_date);
        query.bindValue (":ldate",showTV.last_air_date);
        query.bindValue (":imdb",showTV.imdb_id);
        query.bindValue (":id",id);
// show.reviews
        if(query.exec()){
            // Обновляем название сериала в таблице TVEpisodes
            query.prepare ("UPDATE TVEpisodes SET NameShow=:name WHERE NameShow=:oldName");
            query.bindValue (":name", showTV.nameShow);
            query.bindValue (":oldName", oldName);
            if(query.exec()){
                //Обновляем информацию о Эпизодах
                this->updateTvShowEpisode(showTV);
                // Вносим обзоры в базу
                this->updateReviewsTV(showTV.reviews, showTV.idShow);
                // Обновляем видео ролики
                this->updateVideosTV(showTV.videos, showTV.nameShow, showTV.idShow);
                // Добавляем или обновляем Crew
                this->createUpdateCrewEpisode(showTV);
                // Отправляем сигнал об обновлении главного окна
                emit signalUpdateMainWindowByID("TV", id);
            }else{
                qDebug () << tr("Ошибка при обновлении серии: ") << query.lastError ().text ();
            }

        }else{
            qDebug () << tr("Ошибка при обновлении Сериала: ") << query.lastError ().text ();
        }
    }else{
        qDebug () << tr("Ошибка получении NameShow сериала: ") << query.lastError ().text ();
    }
}

void DBManager::updateMovie(MovieInfo movie, int id)
{
    QSqlQuery query(this->m_database);
    query.prepare("UPDATE Movie SET "
                  "Genre=:genre,"
                  "poster=:poster,"
                  "name=:name,"
                  "Description=:overview,"
                  "idMovie=:idmovie,"
                  "originalLang=:originallang,"
                  "imdbID=:idimdb,"
                  "release_date=:releasedate,"
                  "originalName=:originalname,"
                  "Status=:status "
                  "WHERE id=:id");
    query.bindValue(":genre",movie.genre);
    query.bindValue(":poster",movie.poster);
    query.bindValue(":name",movie.name);
    query.bindValue(":overview",movie.overview);
    query.bindValue(":idmovie",movie.IDMovie);
    query.bindValue(":originallang",movie.originalLang);
    query.bindValue(":idimdb",movie.imdbID);
    query.bindValue(":releasedate",movie.release_date);
    query.bindValue(":originalname",movie.originalName);
    query.bindValue(":status",movie.Status);
    query.bindValue(":id",id);

    if(query.exec()){
        this->updateReviewsTV(movie.reviews, movie.IDMovie);
        // Обновляем видео ролики
        this->updateVideosTV(movie.videos, movie.name, movie.IDMovie);
        // Добавляем или обновляем Crew
        this->createUpdateCrewMovie(movie);
        // Отправляем сигнал об обновлении главного окна
        emit signalUpdateMainWindowByID("Movie", id);
    }else{
        qDebug() << tr("Ошибка обновления фильма: ") << query.lastError().text();
    }

}

void DBManager::saveSettings(QString name, QVariant value)
{
    QSqlQuery query(this->m_database);
    query.prepare("INSERT INTO settings (name, value) VALUES (:name, :value)");
    query.bindValue(":name", name);

    QString data;
    if (value.canConvert<QString>()) {
        data = "string:"+value.toString();
    } else if (value.canConvert<QStringList>()) {
        data = "list:"+value.toStringList().join(";");
    }
    query.bindValue(":value", data);

    if(!query.exec()){
        qDebug() << tr("Ошибка обновления Settings: ") << query.lastError().text();
    }
}

void DBManager::putVault(QString name, QString value)
{
    QSqlQuery query(this->m_database);
    query.prepare("INSERT INTO Vault (name, data) VALUES (:name, :value)");
    query.bindValue(":name", name);
    query.bindValue(":value", value);

    if(!query.exec()){
        qDebug() << tr("Ошибка обновления Vault: ") << query.lastError().text();
    }
}

QHash<QString, QString> DBManager::getVault()
{
    QHash<QString, QString> hash;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT * FROM Vault");
    if(query.exec()){
        while(query.next()){
            hash[query.value("name").toString()]=query.value("data").toString();
        }
    }
    return hash;
}

QVariant  DBManager::getSetting(QString name)
{
    QSqlQuery query(this->m_database);
    query.prepare("SELECT value FROM settings WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        return QVariant();
    }
    if (query.next()) {
        QString value = query.value(0).toString();

        QStringList data = value.split(":");
        QVariant var;
        if(data[0]=="list"){
            QStringList tmp = data[1].split(";");
            var.setValue(tmp);
        }else if(data[0]=="string"){
            var.setValue(data[0]);
        }

        return var;
    } else {
        return QVariant();
    }
}

QMap<QString, QVariant>  DBManager::getAllSettings()
{
    QMap<QString, QVariant>  settings;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT * FROM settings");
    if(query.exec()){
        while (query.next()) {
            QString name = query.value("name").toString();
            QString value = query.value("value").toString();
            QStringList data = value.split(":");
            QVariant var;
            if(data[0]=="list"){
                QStringList tmp = data[1].split(";");
                var.setValue(tmp);
            }else if(data[0]=="string"){
                var.setValue(data[1]);
            }
            settings[name] = var;
        }
    }
    return settings;
}


QString DBManager::getPathToShowTV(QString nameShow)
{
    QString pathToSerial;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT DISTINCT PathToSerial FROM TVEpisodes WHERE NameShow = :nameShow");
    query.bindValue(":nameShow",nameShow);
    if(query.exec()){
        query.next();
        pathToSerial = query.value("PathToSerial").toString();
    }else{
        qDebug() << query.lastError().text();
    }
    return pathToSerial;
}

QString DBManager::getPathToMovie(QString name)
{
    QString path;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT DISTINCT path FROM Movie WHERE name = :name");
    query.bindValue(":name",name);
    if(query.exec()){
        query.next();
        path = query.value("path").toString();
    }else{
        qDebug() << query.lastError().text();
    }
    QFileInfo fileinfo(path);
    return fileinfo.path();
}

QList<int> DBManager::getListNumberSeason(int idDBTVShow)
{
    QSqlQuery query(this->m_database);
    query.prepare("SELECT DISTINCT TVEpisodes.Season FROM TVShow INNER JOIN TVEpisodes ON TVShow.NameShow = TVEpisodes.NameShow WHERE TVShow.ID = :id");
    query.bindValue(":id", idDBTVShow);

    QList<int> list;
    if(query.exec()){
        while(query.next()){
            list.append(query.value("Season").toInt());
        }
    }
    return list;
}

bool DBManager::checkSeasonEpisodeExist(const QString NameShow, const int &season, const int &episode)
{
    QSqlQuery query(this->m_database);
    query.prepare("SELECT 1 FROM TVEpisodes WHERE NameShow = :NameShow AND Season = :season AND Episode = :episode");
    query.bindValue(":NameShow", NameShow);
    query.bindValue(":season", season);
    query.bindValue(":episode", episode);

    if (!query.exec()) {
        qDebug() << tr("Ошибка выполнения запроса:") << query.lastError().text();
        return false;
    }

    return query.next();
}

bool DBManager::updateMovieColumn(const QString &columnName, const QVariant &newValue, int rowId)
{
    QSqlQuery query(this->m_database);
    QString queryString = QString("UPDATE Movie SET %1 = :newValue WHERE id = :rowId").arg(columnName);

    query.prepare(queryString);
    query.bindValue(":newValue", newValue);
    query.bindValue(":rowId", rowId);

    if (query.exec()) {
        return true;
    } else {
        return false;
    }
}

void DBManager::updateShowTVColumn(const QString &columnName, const QVariant &newValue, int rowId)
{
    QSqlQuery query(this->m_database);

    QString queryString = QString("UPDATE TVShow SET %1 = :newValue WHERE ID = :rowId").arg(columnName);
    query.prepare(queryString);
    query.bindValue(":newValue", newValue);
    query.bindValue(":rowId", rowId);

    if (!query.exec()) {
        qDebug() << tr("Ошибка обновления updateShowTVColumn:") << query.lastError().text();
    }
}

bool DBManager::updateEpisodeColumn(const QString &columnName, const QVariant &newValue, int rowId)
{

    QSqlQuery query(this->m_database);
    QString queryString = QString("UPDATE TVEpisodes SET %1 = :newValue WHERE ID = :rowId").arg(columnName);
    query.prepare(queryString);
    query.bindValue(":newValue", newValue);
    query.bindValue(":rowId", rowId);

    if (query.exec()) {
        return true;
    } else {
        return false;
    }
}

void DBManager::updateActorByShowTV(const QString &oldRootPath, const QString &newRootPath, const uint &idShowTV)
{
    QSqlQuery query(this->m_database);
    query.prepare("UPDATE crewEpisode SET "
                  "thumb = REPLACE(thumb, :oldPath, "
                  ":newPath) WHERE idShow = :idShow");
    query.bindValue(":oldPath",oldRootPath);
    query.bindValue(":newPath",newRootPath);
    query.bindValue(":idShow",idShowTV);
    if (!query.exec()) {
        qDebug() << tr("Ошибка обновления crewActor Episode Show:") << query.lastError().text();
    }
/*
 *
;
*/
}


bool DBManager::checkConnectingDB()
{
    if (!this->m_database.open()) {
        qDebug() << tr("Ошибка открытия соединения с базой данных:")
                 << this->m_database.lastError().text();
        return false;
    }

    return true;
}

void DBManager::writeMovieCollectionToDB(QStringList pathlList)
{


    emit signalUpdateProgresBar (tr("Обновляем Базу Данных"));
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
        if(QFile::exists(info.absolutePath()+"/poster.png")){
            poster = info.absolutePath()+"/poster.png";
        }else if(QFile::exists(info.absolutePath()+"/poster.jpg")){
            poster = info.absolutePath()+"/poster.jpg";
        }else{
            poster = ":/images/poster";
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
        query.exec();

        query.finish();
    }
    emit signalUpdateMainWindow ("Movie");
}

void DBManager::writeTVCollectionToDB(QStringList pathlList)
{

    emit signalUpdateProgresBar (tr("Обновляем Базу Данных"));
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
        QString poster = PathToSerial+"/poster.png";
        if(QFile::exists (PathToSerial+"/poster.png")){
            poster = PathToSerial+"/poster.png";
        }else if(QFile::exists (PathToSerial+"/poster.jpg")){
            poster = PathToSerial+"/poster.jpg";
        }else{
            poster = ":/images/poster";
        }

        QString NumSeason = "0";
        QString NumEpisode = "0";

        QRegularExpressionMatch match = DBManager::re.match(fileInfo.fileName ());
        if (match.hasMatch()) {
            NumSeason = match.captured(1);  // Извлекаем номер сезона
            NumEpisode = match.captured(2); // Извлекаем номер серии
        }

        NumSeason = this->removeLeadingZeros(NumSeason);
        NumEpisode = this->removeLeadingZeros(NumEpisode);
        //Проверяем существовании информации о Шоу в целом
        query.prepare("SELECT id FROM TVShow WHERE NameShow = :name");
        query.bindValue(":name", NameShow);
        if(query.exec ()){
            //если информации о шоу нет, мы добавляем информацию о шоу в Базу
            if(!query.next ()){
                query.clear ();
                query.prepare ("INSERT INTO TVShow (NameShow, Overview, Poster) VALUES (:name, :desc, :poster)");
                query.bindValue (":name", NameShow);
                query.bindValue (":desc", "");
                query.bindValue (":poster", poster);
                if (!query.exec()) {
                    qDebug() << query.lastError ().text ();
                }
            }
        }
        query.clear ();

        query.prepare("SELECT id FROM TVEpisodes WHERE File = :File");
        query.bindValue(":File", File);

        if (query.exec()) {
            if(query.next()){
                continue;
            }
        }

        QString posterEpisode = "";

        if(QFile::exists (fileInfo.path()+"/"+NameEpisode+"-thumb.png")){
            posterEpisode = fileInfo.path()+"/"+NameEpisode+"-thumb.png";
        }else if(QFile::exists (fileInfo.path()+"/"+NameEpisode+"-thumb.jpg")){
            posterEpisode = fileInfo.path()+"/"+NameEpisode+"-thumb.jpg";
        }
        query.prepare ("INSERT INTO TVEpisodes (NameShow, PathToSerial, LibraryPath, Overview, File, Episode, Season, NameEpisode, Poster ) "
                                       "VALUES (:name,    :path,       :library,    :desc,    :file, :ep,    :seas,  :NameEpisode, :poster)");
        query.bindValue (":name", NameShow);
        query.bindValue (":path", PathToSerial);
        query.bindValue (":library", LibraryPath);
        query.bindValue (":desc", "");
        query.bindValue (":poster", posterEpisode);
        query.bindValue (":file", File);
        query.bindValue (":ep", NumEpisode);
        query.bindValue (":seas", NumSeason);
        query.bindValue (":NameEpisode", NameEpisode);

        query.exec();
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
    emit signalUpdateProgresBar (tr("Чистим базу от старых записей!"));
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
                        qDebug() << tr("Ошибка удаление старых записей: ") << subQuery.lastError ().text ();
                    }
                }
            }
        }
        query.exec("DELETE FROM Movie WHERE Library_path NOT IN (SELECT path FROM Library);");
    }else if (type==TV) {

        query.prepare("SELECT ID, NameShow, File FROM TVEpisodes");
        if(query.exec ()){
            while(query.next ()){
                if(!QFile::exists (query.value ("File").toString ())){
                    qDebug() << "Удалим: " << query.value ("File").toString ();
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
        query.exec("DELETE FROM TVEpisodes WHERE LibraryPath NOT IN (SELECT path FROM Library)");
        query.exec("DELETE FROM TVShow WHERE NameShow NOT IN (SELECT NameShow FROM TVEpisodes)");
    }
}

GenreList DBManager::loadGenre(QString lang)
{
    GenreList genres;
    QSqlQuery query(this->m_database);
    if(lang=="ru-RU"){
        query.prepare ("SELECT id, ru FROM Genres");
        if(query.exec ()){
            while(query.next()){
                uint id = query.value ("id").toInt();
                QString name = query.value ("ru").toString ();
                if(id>0 && name!=""){
                    genres.addGenre(id, name,"ru");
                }
            }
        }
    }else{
        query.prepare ("SELECT id, en FROM Genres");
        if(query.exec ()){
            while(query.next()){
                uint id = query.value ("id").toInt();
                QString name = query.value ("en").toString ();
                if(id>0 && name!=""){
                    genres.addGenre(id, name,"en");
                }
            }
        }
    }

    return genres;
}

void DBManager::saveGenres(GenreList genres)
{
    QSqlQuery query(this->m_database);
    while(genres.next()){
        genre g = genres.get();
        query.prepare("SELECT 1 FROM Genres WHERE id=:id");
        query.bindValue(":id", g.id);
        query.exec();
        if(query.next()){
            // update
            if(g.lang == "en"){
                query.prepare("UPDATE Genres SET en=:en WHERE id = :id");
                query.bindValue(":en", g.name);
                query.bindValue(":id",g.id);
                query.exec();
            }else{
                query.prepare("UPDATE Genres SET ru=:ru WHERE id = :id");
                query.bindValue(":ru", g.name);
                query.bindValue(":id",g.id);
                query.exec();
            }
        }else{
            // insert
            if(g.lang == "en"){
                query.prepare("INSERT INTO Genres (id, en) VALUES (:id, :en)");
                query.bindValue(":en", g.name);
                query.bindValue(":id",g.id);
                query.exec();
            }else{
                query.prepare("INSERT INTO Genres (id, ru) VALUES (:id, :en)");
                query.bindValue(":ru", g.name);
                query.bindValue(":id",g.id);
                query.exec();
            }
        }
    }
    genres.resetIterator();
}

QList<Videos> DBManager::getVideos(int idShow)
{
    QList<Videos> videos;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT * FROM VideosTV WHERE idShow=:id");
    query.bindValue(":id", idShow);

    if(query.exec()){
        while (query.next()) {
            Videos video;
            video.key = query.value("key").toString();
            video.idShow = query.value("idShow").toInt();
            videos.append(video);
        }
    }
    return videos;
}

QList<Reviews> DBManager::getReviews(int idShow)
{
    QList<Reviews> reviews;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT * FROM ReviewsTV WHERE idShow=:id");
    query.bindValue(":id", idShow);
    if(query.exec()){
        while (query.next()) {
            Reviews review;
            review.author = query.value("author").toString();
            review.content = query.value("content").toString();
            review.nameShow = query.value("nameShow").toString();
            review.idShow = query.value("idShow").toInt();
            reviews.append(review);
        }
    }
    return reviews;
}

// QStringList DBManager::readMovieCollection(QString detailLevel)
// {
//     // qDebug() << "readMovieCollection";
//     const QString DETAIL_LEVEL_ALL = "all";
//     const QString DETAIL_LEVEL_SHORT = "short";
//     QSqlQuery query(this->m_database);
//     QStringList listMovies={};
//     query.prepare("SELECT * FROM Movie");
//     if(query.exec ()){
//         while (query.next()) {
//             QString id = query.value("id").toString();
//             QString poster = query.value("poster").toString();
//             QString name = query.value("name").toString();

//             if (detailLevel == DETAIL_LEVEL_ALL) {
//                 QString genre = query.value("Genre").toString();
//                 QString path = query.value("path").toString();
//                 QString libraryPath = query.value("Library_path").toString();
//                 QString description = query.value("Description").toString();

//                 QString formattedEntry = QString("%1//%2//%3//%4//%5//%6//%7")
//                                              .arg(id, genre, path, poster, name, libraryPath, description);

//                 listMovies.append(formattedEntry);
//             } else if (detailLevel == DETAIL_LEVEL_SHORT) {

//                 QString formattedEntry = QString("%1//%2//%3")
//                                              .arg(id, poster, name);

//                 listMovies.append(formattedEntry);
//             }
//         }
//     }

//     return listMovies;
// }

// QStringList DBManager::readTVCollection(QString detailLevel)
// {
//     const QString DETAIL_LEVEL_ALL = "all";
//     const QString DETAIL_LEVEL_SHORT = "short";

//     QStringList listTV={};
//     //Получаем список сериалов в Базе
//     QSqlQuery queryTVShow(this->m_database);
//     queryTVShow.prepare("SELECT id, NameShow, Poster FROM TVShow");
//     if(queryTVShow.exec ()){
//         //Обрабатываем каждый сериал отдельно
//         while (queryTVShow.next()) {

//             QString Show = ""; // Строка с основной информацией о сериале в целом

//             QString IDShow = queryTVShow.value ("ID").toString ();
//             QString NameShow = queryTVShow.value ("NameShow").toString ();
//             QString PosterShow = queryTVShow.value ("Poster").toString ();
//             Show = QString("%1//%2//%3")
//                                .arg(IDShow, NameShow, PosterShow);
//             Show = Show + "//@//"; //header info о сериале //@// - разделитель между информации о сериале и информации о сериях
//             // Получаем информацию о сериях сериала!
//             QSqlQuery querySeries(this->m_database);
//             querySeries.prepare ("SELECT * FROM TVEpisodes WHERE NameShow = :nameShow");
//             querySeries.bindValue (":nameShow", NameShow);

//             QString body=""; // Строка с основной информацией о сериях сериала
//             if(querySeries.exec ()){
//                 while (querySeries.next ()){

//                     QString IDEpisode = querySeries.value("ID").toString();

//                     QString PathToSerial = querySeries.value("PathToSerial").toString();
//                     QString Poster = querySeries.value("Poster").toString();
//                     QString Episode = querySeries.value("Episode").toString();
//                     QString Season = querySeries.value("Season").toString();
//                     QString File = querySeries.value("File").toString();
//                     QString NameEpisode = querySeries.value("NameEpisode").toString();

//                     if (detailLevel == DETAIL_LEVEL_ALL) { // Полная информация

//                         QString LibraryPath = querySeries.value("LibraryPath").toString();
//                         QString Description = querySeries.value("overview").toString();

//                         body = QString("%1//%2//%3//%4//%5//%6//%7//%8//%9")
//                         .arg(IDEpisode,NameEpisode,PathToSerial, LibraryPath, Description, Poster, File, Season, Episode);
//                     } else if (detailLevel == DETAIL_LEVEL_SHORT) { // Короткая информация

//                         body = QString("%1//%2//%3//%4//%5//%6")
//                         .arg(IDEpisode, NameEpisode, File, Poster, Season, Episode);
//                     }
//                     Show = Show + "#/@/#" + body;
//                 }
//                 listTV.append (Show);
//             }else{
//                 qDebug() << querySeries.lastError ().text ();
//             }


//         }
//     }else{
//         qDebug() << queryTVShow.lastError ().text ();
//     }
//     return listTV;
// }

TVCollection DBManager::getTVCollection()
{
    TVCollection tvcol;
    //Получаем список сериалов в Базе
    QSqlQuery queryTVShow(this->m_database);
    queryTVShow.prepare("SELECT * FROM TVShow");
    if(queryTVShow.exec ()){
        //Обрабатываем каждый сериал отдельно
        while (queryTVShow.next()) {

            ShowInfo showTv;
            showTv.ID = queryTVShow.value ("id").toInt();
            showTv.idShow = queryTVShow.value ("idShow").toInt();
            showTv.nameShow = queryTVShow.value ("NameShow").toString ();
            showTv.poster = queryTVShow.value ("Poster").toString ();
            showTv.overview = queryTVShow.value ("Overview").toString ();
            showTv.originalNameShow = queryTVShow.value ("Original_nameShow").toString ();
            showTv.numberOfEpisodes = queryTVShow.value ("Number_of_episodes").toInt();
            showTv.numberOfSeasons = queryTVShow.value ("Number_of_seasons").toInt();
            showTv.status = queryTVShow.value ("Status").toString();
            showTv.genres = queryTVShow.value ("Genres").toString();
            showTv.production_companies = queryTVShow.value ("production_companies_name").toString();
            showTv.logoPath = queryTVShow.value ("production_companies_logo_path").toString();
            showTv.first_air_date = queryTVShow.value ("first_air_date").toString();
            showTv.last_air_date = queryTVShow.value ("last_air_date").toString();

            // Получаем информацию о сериях сериала!
            QSqlQuery querySeries(this->m_database);
            querySeries.prepare ("SELECT * FROM TVEpisodes WHERE NameShow = :nameShow");
            querySeries.bindValue (":nameShow", showTv.nameShow);

            if(querySeries.exec ()){
                while (querySeries.next ()){
                    EpisodeInfo episode;
                    episode.ID = querySeries.value("ID").toInt();
                    episode.air_date = querySeries.value("air_date").toString();
                    episode.pathToSerial = querySeries.value("PathToSerial").toString();
                    episode.still_path = querySeries.value("Poster").toString();
                    episode.episodeNumber = querySeries.value("Episode").toInt();
                    episode.seasonsNumber = querySeries.value("Season").toInt();
                    episode.filePath = querySeries.value("File").toString();
                    episode.episodeTitle = querySeries.value("NameEpisode").toString();
                    episode.libraryPath = querySeries.value("PathToSerial").toString();
                    episode.overview = querySeries.value("overview").toString();

                    QSqlQuery queryCrew(this->m_database);
                    queryCrew.prepare("SELECT * FROM crewEpisode WHERE idShow=:idShow");
                    queryCrew.bindValue("idShow",showTv.idShow);
                    queryCrew.exec();
                    while(queryCrew.next()){
                        Crew existCrew;
                        existCrew.addRole(queryCrew.value("role").toString());
                        existCrew.id = queryCrew.value("id").toInt();
                        existCrew.name = queryCrew.value("name").toString();
                        existCrew.thumb = queryCrew.value("thumb").toString();
                        episode.addCrew(existCrew);
                    }

                    showTv.addEpisodes(episode);
                }
            }else{
                qDebug() << querySeries.lastError ().text ();
            }
            tvcol.addShow(showTv);
        }
    }else{
        qDebug() << queryTVShow.lastError ().text ();
    }
    return tvcol;
}

MovieCollections DBManager::getMovieCollection()
{
    MovieCollections movCol;
    QSqlQuery query(this->m_database);

    query.prepare("SELECT * FROM Movie");
    if(query.exec ()){
        while (query.next()) {
            MovieInfo movie;
            movie.id = query.value("id").toInt();
            movie.poster = query.value("poster").toString();
            movie.name = query.value("name").toString();
            movie.genre = query.value("Genre").toString();
            movie.path = query.value("path").toString();
            movie.library_path = query.value("Library_path").toString();
            movie.overview = query.value("Description").toString();
            movie.IDMovie = query.value("idMovie").toInt();
            movie.imdbID = query.value("imdbID").toString();
            movie.originalName = query.value("originalName").toString();
            movie.originalLang = query.value("originalLang").toString();
            movie.release_date = query.value("release_date").toString();
            movie.production_companies = query.value("Description").toString();
            movie.logoCompanies = query.value("Description").toString();
            movie.Status = query.value("Status").toString();
            movie.reviews = this->getReviews(movie.IDMovie);
            movie.videos = this->getVideos(movie.IDMovie);
            QSqlQuery queryCrew(this->m_database);
            queryCrew.prepare("SELECT * FROM crewMovie WHERE idMovie=:idMovie");
            queryCrew.bindValue("idMovie",movie.IDMovie);
            queryCrew.exec();
            while(queryCrew.next()){
                Crew existCrew;
                existCrew.addRole(queryCrew.value("role").toString());
                existCrew.id = queryCrew.value("id").toInt();
                existCrew.name = queryCrew.value("name").toString();
                existCrew.thumb = queryCrew.value("thumb").toString();
                movie.addCrew(existCrew);
            }
            movCol.addMovie(movie);
            }
        }
    return movCol;
}

QString DBManager::readTVShowByID(QString detailLevel, int id)
{
    const QString DETAIL_LEVEL_ALL = "all";
    const QString DETAIL_LEVEL_SHORT = "short";

    QString Show = ""; // Строка с основной информацией о сериале в целом

    QSqlQuery queryTVShow(this->m_database);
    queryTVShow.prepare("SELECT id, NameShow, Poster FROM TVShow WHERE id=:id");
    queryTVShow.bindValue (":id", id);
    if(queryTVShow.exec ()){
        if(queryTVShow.next()){

            QString IDShow = queryTVShow.value ("ID").toString ();
            QString NameShow = queryTVShow.value ("NameShow").toString ();
            QString PosterShow = queryTVShow.value ("Poster").toString ();
            Show = QString("%1//%2//%3")
                       .arg(IDShow, NameShow, PosterShow);
            Show = Show + "//@//";
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
                        QString Description = querySeries.value("overview").toString();

                        body = QString("%1//%2//%3//%4//%5//%6//%7//%8//%9")
                                   .arg(IDEpisode,NameEpisode,PathToSerial, LibraryPath, Description, Poster, File, Season, Episode);
                    } else if (detailLevel == DETAIL_LEVEL_SHORT) { // Короткая информация

                        body = QString("%1//%2//%3//%4//%5//%6")
                                   .arg(IDEpisode, NameEpisode, File, Poster, Season, Episode);
                    }
                    Show = Show + "#/@/#" + body;
                }
            }else{
                qDebug() << querySeries.lastError ().text ();
            }
        }
    }
    return Show;
}

ShowInfo DBManager::getShowTVShowByID(int id)
{
    ShowInfo showTv;
    QSqlQuery queryTVShow(this->m_database);
    queryTVShow.prepare("SELECT * FROM TVShow WHERE id=:id");
    queryTVShow.bindValue (":id", id);
    if(queryTVShow.exec ()){
        if(queryTVShow.next()){

            showTv.ID = queryTVShow.value ("id").toInt();
            showTv.idShow = queryTVShow.value ("idShow").toInt();
            showTv.nameShow = queryTVShow.value ("NameShow").toString ();
            showTv.poster = queryTVShow.value ("Poster").toString ();
            showTv.overview = queryTVShow.value ("Overview").toString ();
            showTv.originalNameShow = queryTVShow.value ("Original_nameShow").toString ();
            showTv.numberOfEpisodes = queryTVShow.value ("Number_of_episodes").toInt();
            showTv.numberOfSeasons = queryTVShow.value ("Number_of_seasons").toInt();
            showTv.status = queryTVShow.value ("Status").toString();
            showTv.genres = queryTVShow.value ("Genres").toString();
            showTv.production_companies = queryTVShow.value ("production_companies_name").toString();
            showTv.logoPath = queryTVShow.value ("production_companies_logo_path").toString();
            showTv.first_air_date = queryTVShow.value ("first_air_date").toString();
            showTv.last_air_date = queryTVShow.value ("last_air_date").toString();
            showTv.reviews = this->getReviews(showTv.idShow);
            QSqlQuery querySeries(this->m_database);
            querySeries.prepare ("SELECT * FROM TVEpisodes WHERE NameShow = :nameShow");
            querySeries.bindValue (":nameShow", showTv.nameShow);


            if(querySeries.exec ()){
                while (querySeries.next ()){
                    EpisodeInfo episode;
                    episode.ID = querySeries.value("ID").toInt();
                    episode.air_date = querySeries.value("air_date").toString();
                    episode.pathToSerial = querySeries.value("PathToSerial").toString();
                    episode.still_path = querySeries.value("Poster").toString();
                    episode.episodeNumber = querySeries.value("Episode").toInt();
                    episode.seasonsNumber = querySeries.value("Season").toInt();
                    episode.filePath = querySeries.value("File").toString();
                    episode.episodeTitle = querySeries.value("NameEpisode").toString();
                    episode.libraryPath = querySeries.value("PathToSerial").toString();
                    episode.overview = querySeries.value("overview").toString();
                    episode.episodeID = querySeries.value("idShow").toInt();
                    QSqlQuery queryCrew(this->m_database);
                    queryCrew.prepare("SELECT * FROM crewEpisode WHERE idShow=:idShow");
                    queryCrew.bindValue("idShow",showTv.idShow);
                    queryCrew.exec();
                    while(queryCrew.next()){
                        Crew existCrew;
                        existCrew.addRole(queryCrew.value("role").toString());
                        existCrew.id = queryCrew.value("id").toInt();
                        existCrew.name = queryCrew.value("name").toString();
                        existCrew.thumb = queryCrew.value("thumb").toString();
                        episode.addCrew(existCrew);
                    }
                    showTv.addEpisodes(episode);
                }
            }else{
                qDebug() << querySeries.lastError ().text ();
            }
        }
    }
    return showTv;
}

MovieInfo DBManager::getMovieByID(int id)
{
    MovieInfo movie;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT * FROM Movie WHERE id=:id");
    query.bindValue(":id", id);
    if(query.exec()){
        query.next();
        movie.id = query.value("id").toInt();
        movie.poster = query.value("poster").toString();
        movie.name = query.value("name").toString();
        movie.genre = query.value("Genre").toString();
        movie.path = query.value("path").toString();
        movie.library_path = query.value("Library_path").toString();
        movie.overview = query.value("Description").toString();
        movie.IDMovie = query.value("idMovie").toInt();
        movie.imdbID = query.value("imdbID").toString();
        movie.originalName = query.value("originalName").toString();
        movie.originalLang = query.value("originalLang").toString();
        movie.release_date = query.value("release_date").toString();
        movie.production_companies = query.value("Description").toString();
        movie.logoCompanies = query.value("Description").toString();
        movie.Status = query.value("Status").toString();
        movie.reviews = this->getReviews(movie.IDMovie);
        movie.videos = this->getVideos(movie.IDMovie);

        QSqlQuery queryCrew(this->m_database);
        queryCrew.prepare("SELECT * FROM crewMovie WHERE idMovie=:idMovie");
        queryCrew.bindValue(":idMovie",movie.IDMovie);
        queryCrew.exec();

        while(queryCrew.next()){
            Crew existCrew;
            existCrew.addRole(queryCrew.value("role").toString());
            existCrew.id = queryCrew.value("id").toInt();
            existCrew.name = queryCrew.value("name").toString();
            existCrew.thumb = queryCrew.value("thumb").toString();
            movie.addCrew(existCrew);
        }
    }

    return movie;
}
