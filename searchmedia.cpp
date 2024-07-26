#include "searchmedia.h"
#include "qdir.h"
#include "qurlquery.h"
#include "ui_searchmedia.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QJsonArray>
#include <QFileInfo>

SearchMedia::SearchMedia(QWidget *parent, MediaLibrary *mLib, DBManager *db, DialogShowProgress *sp, Settings *param)
    : QMainWindow(parent)
    , uiSearch(new  Ui::SearchMedia)
    , parametrs(param)
    , mediaLibrary(mLib)
    , dbManager(db)
    , showProgres(sp)
{
    uiSearch->setupUi(this);
    setTypeMediaSearch ("Multi");
    setSearchWord ("");
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    this->genres = new GenreList;
    this->showTv = new ShowInfo;
    this->movie = new MovieInfo;
    // this->oldMovie= new MovieInfo;
    // this->managerNetwork = new QNetworkAccessManager(this);
    uiSearch->viewSearchTree->setColumnWidth(0,128);
    uiSearch->viewSearchTree->setColumnWidth(2,96);
    uiSearch->viewSearchTree->setColumnWidth(3,128);

    connect(uiSearch->selectFindMediaButton, &QPushButton::clicked, this, &SearchMedia::endSelectMedia);
    connect(uiSearch->viewSearchTree, &QTreeWidget::itemSelectionChanged, this, &SearchMedia::slotChangetSelection);

    this->setIdSelectMedia (0);

    QString defLang = param->getSettings("language");
    if(defLang=="en-EN"){
        uiSearch->langSearchComboBox->setCurrentIndex(0);
    }else if(defLang=="ru-RU"){
        uiSearch->langSearchComboBox->setCurrentIndex(1);
    }else{
        uiSearch->langSearchComboBox->setCurrentIndex(0);
    }
    this->progresSearchValue = 0;
    this->progresSearchShow();
}

SearchMedia::~SearchMedia()
{
    disconnect(uiSearch->selectFindMediaButton, &QPushButton::clicked, this, &SearchMedia::endSelectMedia);
    disconnect(uiSearch->viewSearchTree, &QTreeWidget::itemSelectionChanged, this, &SearchMedia::slotChangetSelection);
    delete this->showTv;
    delete genres;
    delete uiSearch;
}
void SearchMedia::on_searchButton_clicked()
{
    if(uiSearch->searchWordEdit->text ()!=""){
        this->progresSearchShow();
        this->progresSearchAdd();
        this->setSearchWord (uiSearch->searchWordEdit->text ());
        this->setLangSearchMedia();
        switch (uiSearch->typeMedia->currentIndex ()) {
        case 0:{this->setTypeMediaSearch ("multi"); break;}
        case 1:{this->setTypeMediaSearch ("movie"); break;}
        case 2:{this->setTypeMediaSearch ("tv"); break;}
        }
        this->progresSearchAdd();
        this->sendRequestTMDBSearch(nameSearch,typeMediaSearch);
    }
}
void SearchMedia::fillFields(QString type)
{
    uiSearch->searchWordEdit->setText (nameSearch);
    if(type=="Movie"){
        this->oldMovie = dbManager->getMovieByID(idMovieDB);
        uiSearch->typeMedia->setCurrentIndex (1);
    }else if(type=="TV"){
        this->oldNameShow = dbManager->getShowTVShowByID(idTVDB);
        uiSearch->typeMedia->setCurrentIndex (2);
    }
}

void SearchMedia::setSearchWord(const QString &newNameSearch)
{
    nameSearch = newNameSearch;
}

void SearchMedia::setTypeMediaSearch(const QString &newTypeMediaSearch)
{
    typeMediaSearch = newTypeMediaSearch;
}

void SearchMedia::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    QWidget::closeEvent(event);
}

bool SearchMedia::getTranslated() const
{
    return translated;
}

void SearchMedia::setTranslated(bool newGetTranslated)
{
    translated = newGetTranslated;
}

QString SearchMedia::getLangSearch() const
{
    return langSearch;
}

void SearchMedia::setLangSearch(const QString &newLangSearch)
{
    langSearch = newLangSearch;
}

void SearchMedia::setLangSearchMedia()
{
    switch(uiSearch->mediaSource->currentIndex()){
        case 0:{
            this->setTranslated(false);
            switch(uiSearch->langSearchComboBox->currentIndex()){
                case 0:{
                    this->setLangSearch("en-EN");
                }break;
                case 1:{
                    this->setTranslated(true);
                    this->setLangSearch("ru-RU");
                }break;
                default: {
                    this->setLangSearch("en-EN");
                }
            }
        }
        break;
    }
}

int SearchMedia::getIdMovieDB() const
{
    return idMovieDB;
}

void SearchMedia::setIdMovieDB(int newIdMovieDB)
{
    idMovieDB = newIdMovieDB;
}

int SearchMedia::getIdTVDB() const
{
    return idTVDB;
}

void SearchMedia::setIdTVDB(int newIdTVDB)
{
    idTVDB = newIdTVDB;
}

QString SearchMedia::getSelectType() const
{
    return selectType;
}

void SearchMedia::setSelectType(const QString &newSelectType)
{
    selectType = newSelectType;
}

int SearchMedia::getIdSelectMedia() const
{
    return idSelectMedia;
}

void SearchMedia::setIdSelectMedia(int newIdSelectMedia)
{
    idSelectMedia = newIdSelectMedia;
}

void SearchMedia::sendRequestTMDBSearch(QString Name, QString type)
{
    qDebug() << "Ищем фильмы... сериалы...";
    this->progresSearchAdd();
    QUrl url(QString("https://api.themoviedb.org/3/search/multi"));

    QUrlQuery query;
    query.addQueryItem("query", Name);
    query.addQueryItem("language", getLangSearch());
    url.setQuery (query);
    QNetworkRequest request(url);

    // Установка заголовков
    request.setRawHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3MzRiMjAxZmI0ZTEzYjM5N2E3ZjBkZWYxMzYxNmRiOSIsIm5iZiI6MTcxOTk0MTM3OC40MjIzNTcsInN1YiI6IjY1OTJlY2QwNjUxZmNmNjA5NjhkYTkzYSIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.zrR3Abjt6k3dIQsdTVsMdUmU-OJCkajxr3KKSadijog");
    request.setRawHeader("accept", "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this,
            [this, type](QNetworkReply *reply) {
        this->slotFinishRequestFindMedia(reply, type);
    });
    manager->get(request);
}


void SearchMedia::sendRequestTMDBSearchGetImage()
{
    qDebug() << "Получаем картинки в окне поиска";

    //https://image.tmdb.org/t/p/original
    //https://image.tmdb.org/t/p/w500/
    this->progresSearchAdd();
    for (int index = 0; index < uiSearch->viewSearchTree->topLevelItemCount(); ++index) {
        this->progresSearchAdd();
        showProgres->updateProgres();
        QTreeWidgetItem *item = uiSearch->viewSearchTree->topLevelItem(index);
        QString posterUrl = item->data(0, Qt::UserRole).toString();
        if(posterUrl==""){
            continue;
        }
        QUrl imageUrl("https://image.tmdb.org/t/p/w154"+posterUrl);

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this,
                [this, index](QNetworkReply *reply) {
                    this->slotUpdateImagesInTree(reply, index);
                });

        QUrl url(imageUrl);
        QNetworkRequest request(url);
        manager->get(request);
    }
}


void SearchMedia::sendRequestTMDBGetInformation(QString lang)
{
    qDebug() << "Получаем информацию о выбранном Фильме/Сериале";
    showProgres->updateProgres();
    QString urlString = QString("https://api.themoviedb.org/3/%1/%2")
                            .arg(getSelectType())
                            .arg(QString::number(getIdSelectMedia()));
    QUrl url(urlString);
    QUrlQuery query;
    // // query.addQueryItem("query", Name);
    // query.addQueryItem("language", "ru-RU");

    query.addQueryItem("append_to_response", "reviews,credits,images,videos,translations,external_ids");
    query.addQueryItem("language", lang);

    url.setQuery (query);
    QNetworkRequest request(url);

    // Установка заголовков
    request.setRawHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3MzRiMjAxZmI0ZTEzYjM5N2E3ZjBkZWYxMzYxNmRiOSIsIm5iZiI6MTcxOTk0MTM3OC40MjIzNTcsInN1YiI6IjY1OTJlY2QwNjUxZmNmNjA5NjhkYTkzYSIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.zrR3Abjt6k3dIQsdTVsMdUmU-OJCkajxr3KKSadijog");
    request.setRawHeader("accept", "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    connect(manager, &QNetworkAccessManager::finished, this, &SearchMedia::slotFinishRequestChooseMedia);

    manager->get(request);

}

void SearchMedia::sendRequestTMDBGetInformationEpisodes(QString lang)
{
    qDebug() << "Получаем информацию о епизодах Сериале";
    countSendRequest = 0;
    showProgres->setMainLineMessage("Получаем подробную информацию о эпизодах Шоу");
    showProgres->updateProgres();
    QList<int> seasons = dbManager->getListNumberSeason(this->idTVDB);
    for (const int& numSeason : seasons) {
        showProgres->updateProgres();
        showProgres->setTextProgres("Запрос на Сезон "+QString::number(numSeason));
        countSendRequest++;
        QString urlString = QString("https://api.themoviedb.org/3/%1/%2/season/%3")
                                .arg(getSelectType())
                                .arg(QString::number(getIdSelectMedia()))
                                .arg(QString::number(numSeason));
        QUrl url(urlString);
        QUrlQuery query;
        // // query.addQueryItem("query", Name);
        // query.addQueryItem("language", "ru-RU");
        query.addQueryItem("language", lang);
        QStringList append_to_response;
        append_to_response.append("credits");
        query.addQueryItem("append_to_response", append_to_response.join(","));
        url.setQuery (query);
        QNetworkRequest request(url);

        // Установка заголовков
        request.setRawHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3MzRiMjAxZmI0ZTEzYjM5N2E3ZjBkZWYxMzYxNmRiOSIsIm5iZiI6MTcxOTk0MTM3OC40MjIzNTcsInN1YiI6IjY1OTJlY2QwNjUxZmNmNjA5NjhkYTkzYSIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.zrR3Abjt6k3dIQsdTVsMdUmU-OJCkajxr3KKSadijog");
        request.setRawHeader("accept", "application/json");

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);

        connect(manager, &QNetworkAccessManager::finished, this, &SearchMedia::slotFinishRequestChooseMediaEpisodes);

        manager->get(request);
    }
}

void SearchMedia::sendRequestTMDBGetImage()
{
    if(getSelectType()=="movie"){
        this->getImageMovie();
    }else if(getSelectType()=="tv"){
        this->getImageTVShow();
    }
    //https://image.tmdb.org/t/p/original
    //https://image.tmdb.org/t/p/w500/

    // for (int index = 0; index < uiSearch->viewSearchTree->topLevelItemCount(); ++index) {
    //     QTreeWidgetItem *item = uiSearch->viewSearchTree->topLevelItem(index);
    //     QString posterUrl = item->data(0, Qt::UserRole).toString();
    //     QUrl imageUrl("https://image.tmdb.org/t/p/w154/"+posterUrl);

    //     QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    //     connect(manager, &QNetworkAccessManager::finished, this,
    //             [this, index](QNetworkReply *reply) {
    //                 this->slotUpdateImagesInTree(reply, index);
    //             });

    //     QUrl url(imageUrl);
    //     QNetworkRequest request(url);
    //     manager->get(request);
    // }
}
void SearchMedia::getImageMovie()
{
    countSendRequest = 0;
    QString name = this->oldMovie.name;
    QString path = dbManager->getPathToMovie(name);
    showProgres->setMainLineMessage("Получаем изображения");
    showProgres->updateProgres();
    if(this->movie->poster.startsWith("/")){
        showProgres->updateProgres();
        countSendRequest++;

        QFileInfo posterFile(this->movie->poster);
        QString namePoster = this->movie->name;
        namePoster = path+"/poster."+posterFile.suffix();
        QUrl imageUrl("https://image.tmdb.org/t/p/original"+this->movie->poster);

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this,
                [this, namePoster,name](QNetworkReply *reply) {
                    this->slotSavePosterFile(reply, namePoster,name);
                });

        QUrl url(imageUrl);
        QNetworkRequest request(url);
        manager->get(request);
        this->movie->poster = namePoster;

    }
    while(this->movie->nextCrew()){
        Crew& crewData = this->movie->getCrew();
        if(crewData.thumb.startsWith("/")){
            QFileInfo fileThumb(crewData.thumb);
            QString namePhotoActor = path+"/actor/"+QString::number(crewData.id)+"."+fileThumb.suffix();
            QFileInfo filePhotoActor(namePhotoActor);
            if(!QFile::exists(filePhotoActor.absolutePath())){
                QDir dir(filePhotoActor.absolutePath());
                dir.mkpath(filePhotoActor.absolutePath());
            }
            QUrl imageUrl("https://image.tmdb.org/t/p/original"+crewData.thumb);
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, &QNetworkAccessManager::finished, this,
                    [this, namePhotoActor,name](QNetworkReply *reply) {
                        this->slotSavePosterFile(reply, namePhotoActor,name);
                    });
            QUrl url(imageUrl);
            QNetworkRequest request(url);
            countSendRequest++;
            manager->get(request);
            crewData.thumb = namePhotoActor;
        }
    }
    // while(episode.nextCrew()){
    //     Crew& crewData = episode.getCrew();
    //     if(crewData.thumb.startsWith("/")){
    //         QFileInfo fileThumb(crewData.thumb);
    //         QString namePhotoActor = pathToShowTV+"/Season "+QString::number(seasonNumber)+"/actor/"+QString::number(crewData.id)+"."+fileThumb.suffix();
    //         QFileInfo filePhotoActor(namePhotoActor);
    //         if(!QFile::exists(filePhotoActor.absolutePath())){
    //             QDir dir(filePhotoActor.absolutePath());
    //             dir.mkpath(filePhotoActor.absolutePath());
    //         }
    //         QUrl imageUrl("https://image.tmdb.org/t/p/original"+crewData.thumb);
    //         QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    //         connect(manager, &QNetworkAccessManager::finished, this,
    //                 [this, namePhotoActor,nameShow](QNetworkReply *reply) {
    //                     this->slotSavePosterFile(reply, namePhotoActor,nameShow);
    //                 });
    //         QUrl url(imageUrl);
    //         QNetworkRequest request(url);
    //         countSendRequest++;
    //         manager->get(request);
    //         crewData.thumb = namePhotoActor;
    //     }
    // }

}

void SearchMedia::getImageTVShow()
{
    countSendRequest = 0;
    QString nameShow = this->oldNameShow.nameShow;
    QString pathToShowTV = dbManager->getPathToShowTV(nameShow);
    showProgres->setMainLineMessage("Получаем изображения");
    showProgres->updateProgres();
    if(this->showTv->poster.startsWith("/")){
        showProgres->updateProgres();
        countSendRequest++;
        QFileInfo posterFile(this->showTv->poster);


        QString namePoster = this->showTv->nameShow;
        namePoster = pathToShowTV+"/poster."+posterFile.suffix();

        QUrl imageUrl("https://image.tmdb.org/t/p/original"+this->showTv->poster);

        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this,
                [this, namePoster,nameShow](QNetworkReply *reply) {
                    this->slotSavePosterFile(reply, namePoster,nameShow);
                });

        QUrl url(imageUrl);
        QNetworkRequest request(url);
        manager->get(request);
        this->showTv->poster = namePoster;

    }
    //Надо обработку доделать, чтобы несколько лого загружало
    // if(this->showTv->logoPath.startsWith("/")){
    //     showProgres->updateProgres();
    //     countSendRequest++;
    //     QFileInfo logoPathFile(this->showTv->logoPath);
    //     QString nameLogo = "production-compaines."+logoPathFile.suffix();
    //     QUrl imageUrl("https://image.tmdb.org/t/p/original"+this->showTv->logoPath);
    //     QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    //     connect(manager, &QNetworkAccessManager::finished, this,
    //             [this, nameLogo, nameShow](QNetworkReply *reply) {
    //                 this->slotSavePosterFile(reply, nameLogo, nameShow);
    //             });

    //     QUrl url(imageUrl);
    //     QNetworkRequest request(url);
    //     manager->get(request);
    //     this->showTv->logoPath = nameLogo;
    // }


    foreach (const uint seasonNumber, this->showTv->Episodes.keys()) {
        QMap<uint, EpisodeInfo>& episodes = this->showTv->Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            showProgres->updateProgres();
            EpisodeInfo& episode = episodes[episodeNumber];
            QFileInfo posterEpisodeFile(episode.still_path);
            QString namePosterEpisode = pathToShowTV+"/Season "+QString::number(seasonNumber)+"/S"+QString::number(episode.seasonsNumber)+"E"+QString::number(episode.episodeNumber)+"-poster."+posterEpisodeFile.suffix();
            QFileInfo filePhotoActor(namePosterEpisode);
            if(!QFile::exists(filePhotoActor.absolutePath())){
                QDir dir(filePhotoActor.absolutePath());
                dir.mkpath(filePhotoActor.absolutePath());
            }

            QUrl imageUrl("https://image.tmdb.org/t/p/original"+episode.still_path);
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, &QNetworkAccessManager::finished, this,
                    [this, namePosterEpisode,nameShow](QNetworkReply *reply) {
                        this->slotSavePosterFile(reply, namePosterEpisode,nameShow);
                    });
            QUrl url(imageUrl);
            QNetworkRequest request(url);
            countSendRequest++;
            manager->get(request);
            episode.still_path = namePosterEpisode;
        }
    }

    while(this->showTv->nextCrew()){
        Crew& crewData = this->showTv->getCrew();
        if(crewData.thumb.startsWith("/")){
            QFileInfo fileThumb(crewData.thumb);
            QString namePhotoActor = pathToShowTV+"/actor/"+QString::number(crewData.id)+"."+fileThumb.suffix();
            QFileInfo filePhotoActor(namePhotoActor);
            if(!QFile::exists(filePhotoActor.absolutePath())){
                QDir dir(filePhotoActor.absolutePath());
                dir.mkpath(filePhotoActor.absolutePath());
            }
            QUrl imageUrl("https://image.tmdb.org/t/p/original"+crewData.thumb);
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, &QNetworkAccessManager::finished, this,
                    [this, namePhotoActor,nameShow](QNetworkReply *reply) {
                        this->slotSavePosterFile(reply, namePhotoActor,nameShow);
                    });
            QUrl url(imageUrl);
            QNetworkRequest request(url);
            countSendRequest++;
            manager->get(request);
            crewData.thumb = namePhotoActor;
        }
    }
    foreach (const uint seasonNumber, this->showTv->Episodes.keys()) {
        QMap<uint, EpisodeInfo>& episodes = this->showTv->Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            showProgres->updateProgres();
            EpisodeInfo& episode = episodes[episodeNumber];
            int seasonNumber = episode.seasonsNumber;
            while(episode.nextCrew()){
                Crew& crewData = episode.getCrew();
                if(crewData.thumb.startsWith("/")){
                    QFileInfo fileThumb(crewData.thumb);
                    QString namePhotoActor = pathToShowTV+"/Season "+QString::number(seasonNumber)+"/actor/"+QString::number(crewData.id)+"."+fileThumb.suffix();
                    QFileInfo filePhotoActor(namePhotoActor);
                    if(!QFile::exists(filePhotoActor.absolutePath())){
                        QDir dir(filePhotoActor.absolutePath());
                        dir.mkpath(filePhotoActor.absolutePath());
                    }
                    QUrl imageUrl("https://image.tmdb.org/t/p/original"+crewData.thumb);
                    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
                    connect(manager, &QNetworkAccessManager::finished, this,
                            [this, namePhotoActor,nameShow](QNetworkReply *reply) {
                                this->slotSavePosterFile(reply, namePhotoActor,nameShow);
                            });
                    QUrl url(imageUrl);
                    QNetworkRequest request(url);
                    countSendRequest++;
                    manager->get(request);
                    crewData.thumb = namePhotoActor;
                }
            }
        }
    }
}

QString SearchMedia::updateField( QString  oldString, QString newString)
{
    QString tmp = newString;

    if(newString!=""){
        return newString;
    }else if(oldString !=""){
        return oldString;
    }else{
        return "-";
    }
}

void SearchMedia::progresSearchAdd()
{
    if(progresSearchValue>=100){
        this->progresSearchValue=0;
    }else{
        this->progresSearchValue+=10;
    }
    uiSearch->progressSearch->setValue(progresSearchValue);
}

void SearchMedia::progresSearchShow()
{
    if(uiSearch->progressSearch->isHidden()){
        uiSearch->progressSearch->show();
    }else{
        uiSearch->progressSearch->hide();
    }
}

void SearchMedia::slotViewOverviewMedia()
{
    this->progresSearchAdd();
    QTreeWidgetItem *selectedItem = uiSearch->viewSearchTree->currentItem();
    QString hiddenData = selectedItem->data(1, Qt::UserRole).toString();
    uiSearch->overviewMedia->setPlainText (hiddenData);
}

void SearchMedia::slotFinishRequestFindMedia(QNetworkReply *reply, QString media_type)
{
    if (reply->error() == QNetworkReply::NoError) {
        this->progresSearchAdd();
        QStringList srcListGenres = dbManager->loadGenre ();
        for (auto& src : srcListGenres) {
            QStringList paraData = src.split (":");
            genres->addGenre (paraData[0].toInt (), paraData[1]);
        }

        // Запрос выполнен успешно:
        QByteArray data = reply->readAll(); // Читаем полученные данные

        QJsonObject jsonObject = QJsonDocument::fromJson(data).object ();

        QJsonArray resultsArray = jsonObject.value("results").toArray();
        int lenghtName = 0;
        QFontMetrics fontMetrics(uiSearch->viewSearchTree->font());
        uiSearch->viewSearchTree->clear ();

        for (const QJsonValue& resultsValue : resultsArray) {
            this->progresSearchAdd();
            QStringList listGenre;
            QJsonObject itemObject = resultsValue.toObject();
            int id = itemObject.value("id").toInt();

            QString type = itemObject.value("media_type").toString();
            if(type!=media_type && media_type!="multi"){
                continue;
            }
            QString name;
            QString original_name;
            QString overview;
            QString poster_path;
            QString date;

            if(type=="tv"){
                name = itemObject.value("name").toString();
                original_name = itemObject.value("original_name").toString();
                date = itemObject.value("first_air_date").toString();

            }else if(type=="movie"){
                name = itemObject.value("title").toString();
                original_name = itemObject.value("original_title").toString();
                date = itemObject.value("release_date").toString();
            }
            if(name.size()==0){
                continue;
            }

            overview = itemObject.value("overview").toString();
            poster_path = itemObject.value("poster_path").toString();
            //Получаем список жанров и переводим их в нормальные названия
            QJsonArray genreIdsArray = itemObject.value("genre_ids").toArray();
            for (const QJsonValue& genreIdValue : genreIdsArray) {
                this->progresSearchAdd();
                if (genreIdValue.isDouble()) { // Проверяем, что значение является числом
                    uint idGenre = genreIdValue.toInt();
                    listGenre.append (genres->getGenre (idGenre));
                }
            }
            QTreeWidgetItem *item = new QTreeWidgetItem(uiSearch->viewSearchTree);
            if(poster_path.startsWith("/")){
                item->setData(0, Qt::UserRole, poster_path); // Poster
            }else{

            QPixmap pixmap;
            pixmap.load ("/opt/MediaFinder/poster.png");
            QLabel *imageLabel = new QLabel();
            imageLabel->setPixmap(pixmap.scaled(90, 128));
            uiSearch->viewSearchTree->setItemWidget(item, 0, imageLabel);

            }
            item->setData(1, Qt::UserRole, overview); // Overview
            item->setData(2, Qt::UserRole, id); // id
            item->setData(3, Qt::UserRole, type); // id
            item->setText(1, name+" \n("+original_name+")"); // Название
            item->setText(2, date); // Год
            item->setText(3, listGenre.join(", ") ); // Жанр
            item->setText(4, type ); // Тип

            int textWidth = fontMetrics.horizontalAdvance(name+"   ");
            lenghtName = std::max(lenghtName, textWidth);
        }
        uiSearch->viewSearchTree->setColumnWidth(1,lenghtName);
        disconnect (uiSearch->viewSearchTree, &QTreeWidget::itemSelectionChanged, this, &SearchMedia::slotViewOverviewMedia);
        connect(uiSearch->viewSearchTree, &QTreeWidget::itemSelectionChanged, this, &SearchMedia::slotViewOverviewMedia);
        this->sendRequestTMDBSearchGetImage();

    } else {
        // Произошла ошибка при выполнении запроса:
        qDebug() << "1Ошибка запроса:" << reply->errorString();
    }
    reply->deleteLater(); // Освобождаем ресурсы ответа
}

void SearchMedia::slotUpdateImagesInTree(QNetworkReply *reply, int index)
{
    if (reply->error() == QNetworkReply::NoError) {
        this->progresSearchAdd();
        showProgres->updateProgres();
        QByteArray imageData = reply->readAll();
        QImage image;
        if (image.loadFromData(imageData)) {
            QTreeWidgetItem *item = uiSearch->viewSearchTree->topLevelItem(index);
            QPixmap pixmap = QPixmap::fromImage(image);
            QLabel *imageLabel = new QLabel();
            imageLabel->setPixmap(pixmap.scaled(90, 128));
            uiSearch->viewSearchTree->setItemWidget(item, 0, imageLabel);

        }
    } else {
        qDebug() << "2Ошибка запроса:" << reply->errorString();
    }
    this->progresSearchShow();
    reply->deleteLater();
}

void SearchMedia::slotChangetSelection()
{
    auto treeWidget = qobject_cast<QTreeWidget*>(sender());
    if (treeWidget->selectedItems().isEmpty()) {
        uiSearch->selectFindMediaButton->setEnabled (false);
        uiSearch->overviewMedia->setPlainText ("");
    }else{
        uiSearch->selectFindMediaButton->setEnabled (true);
        treeWidget->currentItem ();
        QTreeWidgetItem *selectedItem = treeWidget->currentItem();
        int id = selectedItem->data(2, Qt::UserRole).toInt ();
        QString type = selectedItem->data(3, Qt::UserRole).toString ();
        this->setIdSelectMedia (id);
        this->setSelectType (type);

    }
}

void SearchMedia::slotFinishRequestChooseMedia(QNetworkReply *reply)
{
    qDebug() << "Получаем ответ после поиска";
    qDebug() << "getSelectType() " << getSelectType();
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll(); // Читаем полученные данные
        QJsonObject jsonObject = QJsonDocument::fromJson(data).object ();

        if(getSelectType()=="movie"){
            this->processResponseMovie(jsonObject);
            // dbManager->updateMovie(movie);
        }else if(getSelectType()=="tv"){
            this->processResponseTV(jsonObject);
        }

        // dbManager->updateTvShow (*this->show, getIdTVDB());

    } else {
        qDebug() << "3Ошибка запроса:" << reply->errorString();
    }
    showProgres->updateProgres();
    qDebug() << "processResponseMovie";
    reply->deleteLater();
}

void SearchMedia::slotFinishRequestChooseMediaEpisodes(QNetworkReply *reply)
{
    qDebug() << "Получаем ответ о сериях";
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll(); // Читаем полученные данные
        QJsonObject jsonObject = QJsonDocument::fromJson(data).object ();
        // episodes
        QJsonArray episodesArray = jsonObject.value ("episodes").toArray ();
        for (const QJsonValue& episodes : episodesArray) {
            QJsonObject itemEpisodes = episodes.toObject();
            EpisodeInfo *episode = new EpisodeInfo;
            episode->ID = itemEpisodes.value ("id").toInt ();
            episode->episodeNumber = itemEpisodes.value ("episode_number").toInt ();
            episode->seasonsNumber = itemEpisodes.value ("season_number").toInt ();
            if(!this->dbManager->checkSeasonEpisodeExist(episode->seasonsNumber, episode->episodeNumber)){
                continue;
            }
            episode->air_date = itemEpisodes.value ("air_date").toString ();
            episode->still_path = itemEpisodes.value ("still_path").toString ();

            episode->overview = itemEpisodes.value ("overview").toString ();
            episode->episodeTitle = itemEpisodes.value ("name").toString ();

            QJsonArray crewArray = itemEpisodes.value ("crew").toArray();
            for (const QJsonValue& crew : crewArray) {
                QJsonObject crewObject = crew.toObject();
                Crew rawCrew;
                rawCrew.id = crewObject.value("id").toInt();
                rawCrew.name = crewObject.value("name").toString();
                rawCrew.addRole(crewObject.value("job").toString());
                rawCrew.thumb = crewObject.value("profile_path").toString();
                episode->addCrew(rawCrew);
            }

            showProgres->setTextProgres("Ответ на Сезон "+QString::number(episode->seasonsNumber)+" Эпизод "+QString::number(episode->episodeNumber));
            this->showTv->addEpisodes(*episode);
            // if(episode->episodeNumber>0 && episode->seasonsNumber>0){
            // }
            // delete episode;
        }
    } else {
        qDebug() << "4Ошибка запроса:" << reply->errorString();
    }
    showProgres->setTextProgres("");
    qDebug() << "Обновляем базу-1";
    if(countSendRequest==1){

        qDebug() << "Обновляем базу-2";
        if(this->getTranslated()){
            this->setTranslated(false);
            this->sendRequestTMDBGetInformation(getLangSearch());
        }else{
            this->sendRequestTMDBGetImage();
        }

        // this->sendRequestTMDBGetImage();

        // dbManager->updateTvShow (*this->showTv, getIdTVDB());
    }
    countSendRequest--;
    reply->deleteLater();
}

void SearchMedia::slotSavePosterFile(QNetworkReply *reply, QString pathFile, QString nameShow)
{
    if (reply->error() == QNetworkReply::NoError) {
        showProgres->updateProgres();
        QByteArray imageData = reply->readAll();
        // QImage image;
        QFile file(pathFile);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to open file for writing:" << file.errorString();
        }
        // showProgres->setTextProgres("Записываем "+pathFile);
        file.write(imageData);
        file.close();
    } else {
        qDebug() << "5Ошибка запроса:" << reply->errorString();
    }

    if(countSendRequest==1){
        qDebug() << "Обновляем картинки";
        showProgres->setMainLineMessage("Записываем собранную информацию в базу!");
        if(getSelectType()=="movie"){
            dbManager->updateMovie(*movie, getIdMovieDB());
        }else if(getSelectType()=="tv"){
            dbManager->updateTvShow (*this->showTv, getIdTVDB());
        }
        showProgres->setTextProgres("");
        showProgres->setMainLineMessage("Завершено");
        showProgres->closeProgres();
        emit endSearch();
    }
    countSendRequest--;

    reply->deleteLater();
}

void SearchMedia::processResponseTV(QJsonObject jsonObject)
{

    showProgres->updateProgres();
    showProgres->setMainLineMessage("Получаем информацию о выбраном шоу");
    showProgres->setTextProgres("Основная информация");

    this->showTv->idShow = jsonObject.value ("id").toInt ();
    this->showTv->poster = jsonObject.value ("poster_path").toString ();

    this->showTv->nameShow = updateField(this->showTv->nameShow,jsonObject.value ("name").toString ());

    this->showTv->originalNameShow = updateField(this->showTv->originalNameShow,jsonObject.value ("original_name").toString ());
    this->showTv->numberOfEpisodes = jsonObject.value ("number_of_episodes").toInt ();
    this->showTv->numberOfSeasons = jsonObject.value ("number_of_seasons").toInt ();
    this->showTv->overview = updateField(this->showTv->overview,jsonObject.value ("overview").toString ());
    this->showTv->status = jsonObject.value ("status").toString ();
    this->showTv->first_air_date = jsonObject.value ("first_air_date").toString ();
    this->showTv->last_air_date = jsonObject.value ("last_air_date").toString ();

    this->showTv->imdb_id = jsonObject.value ("external_ids").toObject().value("imdb_id").toString();
    QJsonObject credits = jsonObject.value("credits").toObject();

    QJsonArray crewShowTVArray = credits.value ("crew").toArray();
    for (const QJsonValue& crew : crewShowTVArray) {
        QJsonObject crewObject = crew.toObject();
        Crew rawCrew;
        rawCrew.id = crewObject.value("id").toInt();
        rawCrew.name = crewObject.value("name").toString();
        rawCrew.addRole(crewObject.value("job").toString());
        rawCrew.thumb = crewObject.value("profile_path").toString();
        this->showTv->addCrew(rawCrew);
    }
    QJsonArray castShowTVArray = credits.value ("cast").toArray();
    for (const QJsonValue& crew : castShowTVArray) {
        QJsonObject crewObject = crew.toObject();
        Crew rawCrew;
        rawCrew.id = crewObject.value("id").toInt();
        rawCrew.name = crewObject.value("name").toString();
        rawCrew.addRole(crewObject.value("known_for_department").toString());
        rawCrew.thumb = crewObject.value("profile_path").toString();
        this->showTv->addCrew(rawCrew);
    }

    QJsonObject videos = jsonObject.value ("videos").toObject().value("results").toObject();
    for (const QJsonValue& video : videos) {
        showProgres->setTextProgres("YouTube видео");
        QJsonObject itemVideo = video.toObject();
        QString site = itemVideo.value("site").toString();
        if(site == "YouTube") {
            QString key = itemVideo.value("key").toString();
            this->showTv->addVideos(key, this->showTv->idShow);
        }
    }
    QJsonObject jsonReviews = jsonObject.value ("reviews").toObject();
    int countResult = jsonReviews.value("total_results").toInt();
    if (countResult > 0){
        showProgres->setTextProgres("Обзоры от пользователей");
        QJsonArray reviewsResults = jsonReviews.value ("results").toArray ();
        for (const QJsonValue& srcReviewsResults : reviewsResults) {
            QJsonObject itemSrcReviewsResults = srcReviewsResults.toObject();
            QString reviewContent = itemSrcReviewsResults.value("content").toString();
            QString reviewAuthor = itemSrcReviewsResults.value("author").toString();
            Reviews review;
            review.author = reviewAuthor;
            review.content = reviewContent;
            review.nameShow = this->showTv->nameShow;
            review.idShow = this->showTv->idShow;
            this->showTv->addReviews(review);
        }
    }
    QStringList GenreList;
    QJsonArray genres = jsonObject.value ("genres").toArray ();
    for (const QJsonValue& srcGenre : genres) {
        showProgres->setTextProgres("Жанры");
        QJsonObject itemGenre = srcGenre.toObject();
        GenreList.append (itemGenre.value ("name").toString ());
    }
    this->showTv->genres = updateField(this->showTv->genres,GenreList.join (","));
    QJsonArray production_companies = jsonObject.value ("production_companies").toArray ();
    QStringList productionCompanies,logoPath;
    for (const QJsonValue& srcProduction_companies : production_companies) {
        showProgres->setTextProgres("Компания производитель");
        QJsonObject itemProduction_companies= srcProduction_companies.toObject();
        logoPath.append (itemProduction_companies.value ("logo_path").toString ());
        productionCompanies.append (itemProduction_companies.value ("name").toString ());
    }
    this->showTv->production_companies = productionCompanies.join (",");
    this->showTv->logoPath = logoPath.join (",");
    qDebug() << "Данные о сериале получены";
    showProgres->setTextProgres("");
    // Первый раз получаем информацию на английском
    // Второй раз на языке локализации выбраной
    if(this->getTranslated()){
        sendRequestTMDBGetInformationEpisodes();
    }else{
        sendRequestTMDBGetInformationEpisodes(getLangSearch());
    }
}

void SearchMedia::processResponseMovie(QJsonObject jsonObject)
{
    showProgres->updateProgres();
    showProgres->setMainLineMessage("Получаем информацию о выбраном фильме");
    showProgres->setTextProgres("Основная информация");


    movie->IDMovie = jsonObject.value("id").toInt();
    movie->imdbID = jsonObject.value ("external_ids").toObject().value("imdb_id").toString();

    movie->overview = this->updateField(movie->overview, jsonObject.value("overview").toString());
    movie->originalName = this->updateField(movie->originalName, jsonObject.value("original_title").toString());
    movie->name = this->updateField(movie->name, jsonObject.value("title").toString());

    movie->originalLang = this->updateField(movie->originalLang,jsonObject.value("original_language").toString());
    movie->poster = this->updateField(movie->poster,jsonObject.value("poster_path").toString());
    movie->release_date = this->updateField(movie->release_date,jsonObject.value("release_date").toString());
    movie->Status = this->updateField(movie->Status,jsonObject.value("status").toString());

    if(this->movie->getSizeCredits()==0){
        QJsonObject credits = jsonObject.value("credits").toObject();

        QJsonArray crewMovieArray = credits.value ("crew").toArray();
        for (const QJsonValue& crew : crewMovieArray) {
            QJsonObject crewObject = crew.toObject();
            Crew rawCrew;
            rawCrew.id = crewObject.value("id").toInt();
            rawCrew.name = crewObject.value("name").toString();
            rawCrew.addRole(crewObject.value("job").toString());
            rawCrew.thumb = crewObject.value("profile_path").toString();
            this->movie->addCrew(rawCrew);
        }
        QJsonArray castMovieArray = credits.value ("cast").toArray();
        for (const QJsonValue& crew : castMovieArray) {
            QJsonObject crewObject = crew.toObject();
            Crew rawCrew;
            rawCrew.id = crewObject.value("id").toInt();
            rawCrew.name = crewObject.value("name").toString();
            rawCrew.addRole(crewObject.value("known_for_department").toString());
            rawCrew.thumb = crewObject.value("profile_path").toString();
            this->movie->addCrew(rawCrew);
        }
    }

    QStringList GenreList;
    QJsonArray genres = jsonObject.value ("genres").toArray ();
    for (const QJsonValue& srcGenre : genres) {
        showProgres->setTextProgres("Жанры");
        QJsonObject itemGenre = srcGenre.toObject();
        GenreList.append (itemGenre.value ("name").toString ());
    }
    movie->genre = this->updateField(movie->genre, GenreList.join(","));

    QJsonArray production_companies = jsonObject.value ("production_companies").toArray ();
    QStringList productionCompanies,logoPath;
    for (const QJsonValue& srcProduction_companies : production_companies) {
        showProgres->setTextProgres("Компания производитель");
        QJsonObject itemProduction_companies= srcProduction_companies.toObject();
        logoPath.append (itemProduction_companies.value ("logo_path").toString ());
        productionCompanies.append (itemProduction_companies.value ("name").toString ());
    }
    movie->production_companies = productionCompanies.join(",");
    movie->logoCompanies = logoPath.join(",");

    QJsonObject jsonReviews = jsonObject.value ("reviews").toObject();
    int countResult = jsonReviews.value("total_results").toInt();
    if (countResult > 0){
        showProgres->setTextProgres("Обзоры от пользователей");
        QJsonArray reviewsResults = jsonReviews.value ("results").toArray ();
        for (const QJsonValue& srcReviewsResults : reviewsResults) {
            QJsonObject itemSrcReviewsResults = srcReviewsResults.toObject();
            QString reviewContent = itemSrcReviewsResults.value("content").toString();
            QString reviewAuthor = itemSrcReviewsResults.value("author").toString();
            Reviews review;
            review.author = reviewAuthor;
            review.content = reviewContent;
            review.nameShow = this->movie->originalName;
            review.idShow = movie->IDMovie;
            this->movie->addReviews(review);
        }
    }
    if(this->getTranslated()){
        this->setTranslated(false);
        this->sendRequestTMDBGetInformation(getLangSearch());
    }else{
        this->sendRequestTMDBGetImage();
    }

}

void SearchMedia::endSelectMedia()
{
    showProgres->updateProgres();
    showProgres->show();
    qDebug() << "Close end search and select media!";
    // Отправляем запрос для загрузки информации о фильме/сериале
    this->sendRequestTMDBGetInformation();
}


void SearchMedia::on_selectFindMediaButton_clicked()
{
    this->close ();
    emit selectMedia();

    //Кнопка только закрывает окно
    //А загрузка данных происходит по событию Click и вызывает endSelectMedia();
}

