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

void DBManager::updateReviewsTV(QList<Reviews> reviews, QString NameShow)
{
    QSqlQuery query(this->m_database);
    //Удаляем старые обзоры перед обновлением
    query.prepare("DELETE FROM ReviewsTV WHERE nameShow = :nameShow");
    query.bindValue(":nameShow", NameShow);
    query.exec();

    for (const auto& review : reviews) {
        query.clear();
        // QString q = "INSERT INTO ReviewsTV (nameShow, author, content) VALUE ("+review.nameShow+", "+review.nameShow+", "+review.nameShow+"")";
        query.prepare("INSERT INTO ReviewsTV (nameShow, author, content) VALUES (:name, :a, :c)");
        query.bindValue(":name", review.nameShow);
        query.bindValue(":a", review.author);
        query.bindValue(":c", review.content);
        if(!query.exec()){
            qDebug() << "1-1 Ошибка выполнения запроса:" << query.lastError().text();
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
            qDebug() << "Обновляем серии";

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
                qDebug() << "2Ошибка выполнения запроса:" << query.lastError().text();
            }
        }
    }
}

void DBManager::updateVideosTV(QList<Videos> videos, QString NameShow)
{
    QSqlQuery query(this->m_database);
    //Удаляем старые видео перед обновлением
    query.prepare("DELETE FROM VideosTV WHERE nameShow = :nameShow");
    query.bindValue(":nameShow", NameShow);
    query.exec();

    for (const Videos& video : videos) {
        query.prepare("INSERT INTO VideosTV (nameShow, key) VALUES (:nameShow, :key)");
        query.bindValue("nameShow",NameShow);
        query.bindValue("key",video.key);
        if(!query.exec()){
            qDebug() << "3Ошибка выполнения запроса:" << query.lastError().text();
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

void DBManager::updateTvShow(ShowInfo show, int id)
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
                      "production_companies_name=:proName,"
                      "production_companies_logo_path=:logo,"
                      "IdShow=:idShow WHERE id=:id");
        query.bindValue (":nameShow",show.nameShow);
        query.bindValue (":overview",show.overview);
        query.bindValue (":poster",show.poster);
        query.bindValue (":originalNameShow",show.originalNameShow);
        query.bindValue (":numberOfEpisodes",show.numberOfEpisodes);
        query.bindValue (":numberOfSeasons",show.numberOfSeasons);
        query.bindValue (":status",show.status);
        query.bindValue (":genres",show.genres);
        query.bindValue (":IdShow",show.idShow);
        query.bindValue (":proName",show.production_companies);
        query.bindValue (":logo",show.logoPath);
        query.bindValue (":id",id);
// show.reviews
        if(query.exec()){
            // Обновляем название сериала в таблице TVEpisodes
            query.prepare ("UPDATE TVEpisodes SET NameShow=:name WHERE NameShow=:oldName");
            query.bindValue (":name", show.nameShow);
            query.bindValue (":oldName", oldName);
            if(query.exec()){
                //Обновляем информацию о Эпизодах
                this->updateTvShowEpisode(show);
                // Вносим обзоры в базу
                this->updateReviewsTV(show.reviews, show.nameShow);
                // Обновляем видео ролики
                this->updateVideosTV(show.videos, show.nameShow);

                // Отправляем сигнал об обновлении главного окна
                emit signalUpdateMainWindowByID("TV", id);
            }else{
                qDebug () << query.lastError ().text ();
            }
        }else{
            qDebug () << query.lastError ().text ();
        }
    }else{
        qDebug () << query.lastError ().text ();
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
    query.bindValue(":status,",movie.Status);
    query.bindValue(":id",id);

    if(query.exec()){
        this->updateReviewsTV(movie.reviews, movie.name);
        // Обновляем видео ролики
        this->updateVideosTV(movie.videos, movie.name);
        // Отправляем сигнал об обновлении главного окна
        emit signalUpdateMainWindowByID("Movie", id);
    }else{
        qDebug() << "Update Movie Error: " << query.lastError().text();
    }

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
        // if(posterFile.exists()){
        //     poster = posterFile.canonicalFilePath();
        // }else{
        //     poster = "/opt/MediaFinder/poster.png";
        // }
        poster = posterFile.canonicalFilePath();
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
        // if(QFile::exists (PathToSerial+"/poster.png")){
        //     poster = PathToSerial+"/poster.png";
        // }else{
        //     // Или пишем стандартный постер
        //     poster = "/opt/MediaFinder/poster.png";
        // }
        poster = PathToSerial+"/poster.png";
        QString NumSeason = "0";
        QString NumEpisode = "0";

        QRegularExpressionMatch match = DBManager::re.match(fileInfo.fileName ());
        if (match.hasMatch()) {
            NumSeason = match.captured(1);  // Извлекаем номер сезона
            NumEpisode = match.captured(2); // Извлекаем номер серии
        }
        NumSeason = this->removeLeadingZeros(NumSeason);
        NumEpisode = this->removeLeadingZeros(NumEpisode);
        query.prepare("SELECT id FROM TVShow WHERE NameShow = :name");
        query.bindValue(":name", NameShow);
        if(query.exec ()){
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
        // INSERT INTO TVShow (NameShow, Overview, Poster) VALUES (:name, :desc, :poster);

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

        query.prepare ("INSERT INTO TVEpisodes (NameShow, PathToSerial, LibraryPath, Overview, File, Episode, Season, NameEpisode ) "
                                       "VALUES (:name,    :path,       :library,    :desc,    :file, :ep,    :seas,  :NameEpisode)");
        query.bindValue (":name", NameShow);
        query.bindValue (":path", PathToSerial);
        query.bindValue (":library", LibraryPath);
        query.bindValue (":desc", "");
        // query.bindValue (":poster", "/opt/MediaFinder/poster.png");
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

QList<Videos> DBManager::getVideos(QString name)
{
    QList<Videos> videos;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT * FROM VideosTV WHERE NameShow=:name");
    query.bindValue(":name", name);

    if(query.exec()){
        while (query.next()) {
            Videos video;
            video.key = query.value("key").toString();
            videos.append(video);
        }
    }
    return videos;
}

QList<Reviews> DBManager::getReviews(QString name)
{
    QList<Reviews> reviews;
    QSqlQuery query(this->m_database);
    query.prepare("SELECT * FROM ReviewsTV WHERE NameShow=:name");
    query.bindValue(":name", name);
    if(query.exec()){
        while (query.next()) {
            Reviews review;
            review.author = query.value("author").toString();
            review.content = query.value("content").toString();
            review.nameShow = query.value("nameShow").toString();
            reviews.append(review);
        }
    }
    return reviews;
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
                        QString Description = querySeries.value("overview").toString();

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

TVCollection DBManager::getTVCollection()
{
    TVCollection tvcol;
    //Получаем список сериалов в Базе
    QSqlQuery queryTVShow(this->m_database);
    queryTVShow.prepare("SELECT * FROM TVShow");
    if(queryTVShow.exec ()){
        //Обрабатываем каждый сериал отдельно
        while (queryTVShow.next()) {

            ShowInfo tvShow;
            tvShow.ID = queryTVShow.value ("id").toInt();
            tvShow.idShow = queryTVShow.value ("idShow").toInt();
            tvShow.nameShow = queryTVShow.value ("NameShow").toString ();
            tvShow.poster = queryTVShow.value ("Poster").toString ();
            tvShow.overview = queryTVShow.value ("Overview").toString ();
            tvShow.originalNameShow = queryTVShow.value ("Original_nameShow").toString ();
            tvShow.numberOfEpisodes = queryTVShow.value ("Number_of_episodes").toInt();
            tvShow.numberOfSeasons = queryTVShow.value ("Number_of_seasons").toInt();
            tvShow.status = queryTVShow.value ("Status").toString();
            tvShow.genres = queryTVShow.value ("Genres").toString();
            tvShow.production_companies = queryTVShow.value ("production_companies_name").toString();
            tvShow.logoPath = queryTVShow.value ("production_companies_logo_path").toString();

            // Получаем информацию о сериях сериала!
            QSqlQuery querySeries(this->m_database);
            querySeries.prepare ("SELECT * FROM TVEpisodes WHERE NameShow = :nameShow");
            querySeries.bindValue (":nameShow", tvShow.nameShow);

            if(querySeries.exec ()){
                while (querySeries.next ()){
                    EpisodeInfo episode;

                    episode.ID = querySeries.value("ID").toInt();

                    episode.pathToSerial = querySeries.value("PathToSerial").toString();
                    episode.still_path = querySeries.value("Poster").toString();
                    episode.episodeNumber = querySeries.value("Episode").toInt();
                    episode.seasonsNumber = querySeries.value("Season").toInt();
                    episode.filePath = querySeries.value("File").toString();
                    episode.episodeTitle = querySeries.value("NameEpisode").toString();
                    episode.libraryPath = querySeries.value("LibraryPath").toString();
                    episode.overview = querySeries.value("overview").toString();

                    tvShow.addEpisodes(episode);
                }
            }else{
                qDebug() << querySeries.lastError ().text ();
            }
            tvcol.addShow(tvShow);
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
            movie.imdbID = query.value("imdbID").toInt();
            movie.originalName = query.value("originalName").toString();
            movie.originalLang = query.value("originalLang").toString();
            movie.release_date = query.value("release_date").toString();
            movie.production_companies = query.value("Description").toString();
            movie.logoCompanies = query.value("Description").toString();
            movie.Status = query.value("Status").toString();
            movie.reviews = this->getReviews(movie.name);
            movie.videos = this->getVideos(movie.name);
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

            QSqlQuery querySeries(this->m_database);
            querySeries.prepare ("SELECT * FROM TVEpisodes WHERE NameShow = :nameShow");
            querySeries.bindValue (":nameShow", showTv.nameShow);


            if(querySeries.exec ()){
                while (querySeries.next ()){
                    EpisodeInfo episode;
                    episode.ID = querySeries.value("ID").toInt();
                    episode.pathToSerial = querySeries.value("PathToSerial").toString();
                    episode.still_path = querySeries.value("Poster").toString();
                    episode.episodeNumber = querySeries.value("Episode").toInt();
                    episode.seasonsNumber = querySeries.value("Season").toInt();
                    episode.filePath = querySeries.value("File").toString();
                    episode.episodeTitle = querySeries.value("NameEpisode").toString();
                    episode.libraryPath = querySeries.value("LibraryPath").toString();
                    episode.overview = querySeries.value("overview").toString();
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
        movie.imdbID = query.value("imdbID").toInt();
        movie.originalName = query.value("originalName").toString();
        movie.originalLang = query.value("originalLang").toString();
        movie.release_date = query.value("release_date").toString();
        movie.production_companies = query.value("Description").toString();
        movie.logoCompanies = query.value("Description").toString();
        movie.Status = query.value("Status").toString();
        movie.reviews = this->getReviews(movie.name);
        movie.videos = this->getVideos(movie.name);
    }else{
        qDebug() << "getMovieByID: " << query.lastError().text();
    }

    return movie;
}







