#include "searchmedia.h"
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

SearchMedia::SearchMedia(QWidget *parent, MediaLibrary *mLib, DBManager *db)
    : QMainWindow(parent)
    , uiSearch(new  Ui::SearchMedia)
    , mediaLibrary(mLib)
    , dbManager(db)
{
    uiSearch->setupUi(this);
    setTypeMediaSearch ("Multi");
    setSearchWord ("");
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    this->genres = new GenreList;
    this->showTv = new ShowInfo;
    // this->managerNetwork = new QNetworkAccessManager(this);
    uiSearch->viewSearchTree->setColumnWidth(0,128);
    uiSearch->viewSearchTree->setColumnWidth(2,96);
    uiSearch->viewSearchTree->setColumnWidth(3,128);

    connect(uiSearch->okButton, &QPushButton::clicked, this, &SearchMedia::endSelectMedia);
    connect(uiSearch->viewSearchTree, &QTreeWidget::itemSelectionChanged, this, &SearchMedia::slotChangetSelection);

    this->setIdSelectMedia (0);

}

SearchMedia::~SearchMedia()
{
    // delete genres;
    delete uiSearch;
}
void SearchMedia::on_searchButton_clicked()
{
    if(uiSearch->searchWordEdit->text ()!=""){
        this->setSearchWord (uiSearch->searchWordEdit->text ());

        switch (uiSearch->typeMedia->currentIndex ()) {
        case 0:{this->setTypeMediaSearch ("multi"); break;}
        case 1:{this->setTypeMediaSearch ("movie"); break;}
        case 2:{this->setTypeMediaSearch ("tv"); break;}
        }

        this->sendRequestTMDBSearch(nameSearch,typeMediaSearch);
    }
}
void SearchMedia::fillFields(QString type)
{
    uiSearch->searchWordEdit->setText (nameSearch);
    if(type=="Movie"){
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
    QUrl url(QString("https://api.themoviedb.org/3/search/multi"));

    QUrlQuery query;
    query.addQueryItem("query", Name);
    query.addQueryItem("language", "ru-RU");
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

    for (int index = 0; index < uiSearch->viewSearchTree->topLevelItemCount(); ++index) {
        QTreeWidgetItem *item = uiSearch->viewSearchTree->topLevelItem(index);
        QString posterUrl = item->data(0, Qt::UserRole).toString();
        QUrl imageUrl("https://image.tmdb.org/t/p/w154/"+posterUrl);

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

void SearchMedia::sendRequestTMDBGetInformation()
{
    qDebug() << "Получаем информацию о выбранном Фильме/Сериале";
    QString urlString = QString("https://api.themoviedb.org/3/%1/%2")
                            .arg(getSelectType())
                            .arg(QString::number(getIdSelectMedia()));
    QUrl url(urlString);
    QUrlQuery query;
    // // query.addQueryItem("query", Name);
    // query.addQueryItem("language", "ru-RU");

    query.addQueryItem("append_to_response", "reviews%2Ccredits%2Cimages%2Cvideos%2Ctranslations");
    query.addQueryItem("language", "en-EN");
    url.setQuery (query);
    QNetworkRequest request(url);

    // Установка заголовков
    request.setRawHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3MzRiMjAxZmI0ZTEzYjM5N2E3ZjBkZWYxMzYxNmRiOSIsIm5iZiI6MTcxOTk0MTM3OC40MjIzNTcsInN1YiI6IjY1OTJlY2QwNjUxZmNmNjA5NjhkYTkzYSIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.zrR3Abjt6k3dIQsdTVsMdUmU-OJCkajxr3KKSadijog");
    request.setRawHeader("accept", "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    connect(manager, &QNetworkAccessManager::finished, this, &SearchMedia::slotFinishRequestChooseMedia);

    manager->get(request);

}

void SearchMedia::sendRequestTMDBGetInformationEpisodes(int count)
{
    qDebug() << "Получаем информацию о выбранном Фильме/Сериале";
    countSendRequest = 0;
    for (int index = 1; index <= count; ++index) {
        countSendRequest++;
        qDebug() << countSendRequest;
        QString urlString = QString("https://api.themoviedb.org/3/%1/%2/season/%3")
                                .arg(getSelectType())
                                .arg(QString::number(getIdSelectMedia()))
                                .arg(QString::number(index));
        QUrl url(urlString);
        QUrlQuery query;
        // // query.addQueryItem("query", Name);
        // query.addQueryItem("language", "ru-RU");
        query.addQueryItem("language", "en-EN");
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
    //https://image.tmdb.org/t/p/original
    //https://image.tmdb.org/t/p/w500/
    countSendRequest = 0;
    QString nameShow = this->oldNameShow.nameShow;
    QString pathToShowTV = dbManager->getPathToShowTV(nameShow);
    if(this->showTv->poster.startsWith("/")){
        countSendRequest++;
        QFileInfo posterFile(this->showTv->poster);


        QString namePoster = this->showTv->nameShow;
        namePoster = pathToShowTV+"/"+namePoster.replace(" ","-")+"."+posterFile.suffix();

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
    if(this->showTv->logoPath.startsWith("/")){
        countSendRequest++;
        QFileInfo logoPathFile(this->showTv->logoPath);
        QString nameLogo = "production-compaines."+logoPathFile.suffix();
        QUrl imageUrl("https://image.tmdb.org/t/p/original"+this->showTv->logoPath);
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this,
                [this, nameLogo,nameShow](QNetworkReply *reply) {
                    this->slotSavePosterFile(reply, nameLogo,nameShow);
                });

        QUrl url(imageUrl);
        QNetworkRequest request(url);
        manager->get(request);
        this->showTv->logoPath = nameLogo;
    }


    foreach (const uint seasonNumber, this->showTv->Episodes.keys()) {
        QMap<uint, EpisodeInfo>& episodes = this->showTv->Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            EpisodeInfo& episode = episodes[episodeNumber];
            QFileInfo posterEpisodeFile(episode.still_path);
            QString namePosterEpisode = pathToShowTV+"/S"+QString::number(episode.seasonsNumber)+"E"+QString::number(episode.episodeNumber)+"-poster."+posterEpisodeFile.suffix();


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

void SearchMedia::slotViewOverviewMedia()
{
    QTreeWidgetItem *selectedItem = uiSearch->viewSearchTree->currentItem();
    QString hiddenData = selectedItem->data(1, Qt::UserRole).toString();
    uiSearch->overviewMedia->setPlainText (hiddenData);
}

void SearchMedia::slotFinishRequestFindMedia(QNetworkReply *reply, QString media_type)
{
    if (reply->error() == QNetworkReply::NoError) {
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


            overview = itemObject.value("overview").toString();
            poster_path = itemObject.value("poster_path").toString();
            //Получаем список жанров и переводим их в нормальные названия
            QJsonArray genreIdsArray = itemObject.value("genre_ids").toArray();
            for (const QJsonValue& genreIdValue : genreIdsArray) {
                if (genreIdValue.isDouble()) { // Проверяем, что значение является числом
                    uint idGenre = genreIdValue.toInt();
                    listGenre.append (genres->getGenre (idGenre));
                }
            }
            QTreeWidgetItem *item = new QTreeWidgetItem(uiSearch->viewSearchTree);
            item->setData(0, Qt::UserRole, poster_path); // Poster
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
        qDebug() << "Ошибка запроса:" << reply->errorString();
    }
    reply->deleteLater(); // Освобождаем ресурсы ответа
}

void SearchMedia::slotUpdateImagesInTree(QNetworkReply *reply, int index)
{
    if (reply->error() == QNetworkReply::NoError) {
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
        qDebug() << "Ошибка запроса:" << reply->errorString();
    }
    reply->deleteLater();
}

void SearchMedia::slotChangetSelection()
{
    auto treeWidget = qobject_cast<QTreeWidget*>(sender());
    if (treeWidget->selectedItems().isEmpty()) {
        uiSearch->okButton->setEnabled (false);
        uiSearch->overviewMedia->setPlainText ("");
    }else{
        uiSearch->okButton->setEnabled (true);
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
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll(); // Читаем полученные данные
        QJsonObject jsonObject = QJsonDocument::fromJson(data).object ();
        this->showTv->idShow = jsonObject.value ("id").toInt ();
        this->showTv->poster = jsonObject.value ("poster_path").toString ();
        this->showTv->nameShow = jsonObject.value ("name").toString ();
        this->showTv->originalNameShow = jsonObject.value ("original_name").toString ();
        this->showTv->numberOfEpisodes = jsonObject.value ("number_of_episodes").toInt ();
        this->showTv->numberOfSeasons = jsonObject.value ("number_of_seasons").toInt ();
        this->showTv->overview = jsonObject.value ("overview").toString ();
        this->showTv->status = jsonObject.value ("status").toString ();

        QJsonObject videos = jsonObject.value ("videos").toObject().value("results").toObject();
        for (const QJsonValue& video : videos) {
            QJsonObject itemVideo = video.toObject();
            QString site = itemVideo.value("site").toString();
            if(site == "YouTube") {
                QString key = itemVideo.value("key").toString();
                this->showTv->addVideos(key);
            }
        }
        QJsonObject jsonReviews = jsonObject.value ("reviews").toObject();
        int countResult = jsonReviews.value("total_results").toInt();
        if (countResult > 0){

            QJsonArray reviewsResults = jsonReviews.value ("results").toArray ();
            for (const QJsonValue& srcReviewsResults : reviewsResults) {
                QJsonObject itemSrcReviewsResults = srcReviewsResults.toObject();
                QString reviewContent = itemSrcReviewsResults.value("content").toString();
                QString reviewAuthor = itemSrcReviewsResults.value("author").toString();
                Reviews review;
                review.author = reviewAuthor;
                review.author = reviewContent;
                review.nameShow = this->showTv->nameShow;
                this->showTv->addReviews(review);
            }

        }

        // QJsonArray seasons = jsonObject.value ("seasons").toArray ();
        // for (const QJsonValue& srcSeason : seasons) {
        //     QJsonObject itemSeason = srcSeason.toObject();
        //     int season_number = itemSeason.value("season_number").toInt ();
        //     int episode_count = itemSeason.value("episode_count").toInt ();
        //     QString idSeason = itemSeason.value("id").toString ();
        // }
        QStringList GenreList;
        QJsonArray genres = jsonObject.value ("genres").toArray ();
        for (const QJsonValue& srcGenre : genres) {
            QJsonObject itemGenre = srcGenre.toObject();
            GenreList.append (itemGenre.value ("name").toString ());

        }
        this->showTv->genres = GenreList.join (",");
        QJsonArray production_companies = jsonObject.value ("production_companies").toArray ();
        QStringList productionCompanies,logoPath;
        for (const QJsonValue& srcProduction_companies : production_companies) {
            QJsonObject itemProduction_companies= srcProduction_companies.toObject();
            logoPath.append (itemProduction_companies.value ("logo_path").toString ());
            productionCompanies.append (itemProduction_companies.value ("name").toString ());
        }
        this->showTv->production_companies = productionCompanies.join (",");
        this->showTv->logoPath = logoPath.join (",");
        qDebug() << "Данные о сериале получены";
        sendRequestTMDBGetInformationEpisodes(this->showTv->numberOfSeasons);

        // dbManager->updateTvShow (*this->show, getIdTVDB());

    } else {
        qDebug() << "Ошибка запроса:" << reply->errorString();
    }
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
            episode->overview = itemEpisodes.value ("overview").toString ();
            episode->episodeTitle = itemEpisodes.value ("name").toString ();
            episode->air_date = itemEpisodes.value ("air_date").toString ();
            episode->still_path = itemEpisodes.value ("still_path").toString ();
            if(episode->episodeNumber>0 && episode->seasonsNumber>0){
               this->showTv->addEpisodes(*episode);
            }
            delete episode;
        }
    } else {
        qDebug() << "Ошибка запроса:" << reply->errorString();
    }

    qDebug() << "Обновляем базу-1";
    if(countSendRequest==1){
        qDebug() << "Обновляем базу-2";
        this->sendRequestTMDBGetImage();

        // dbManager->updateTvShow (*this->showTv, getIdTVDB());
    }
    countSendRequest--;
    reply->deleteLater();
}

void SearchMedia::slotSavePosterFile(QNetworkReply *reply, QString pathFile, QString nameShow)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        QImage image;
        QFile file(pathFile);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to open file for writing:" << file.errorString();
        }
        file.write(imageData);
        file.close();
    } else {
        qDebug() << "Ошибка запроса:" << reply->errorString();
    }

    if(countSendRequest==1){
        qDebug() << "Обновляем картинки";
        dbManager->updateTvShow (*this->showTv, getIdTVDB());
    }
    countSendRequest--;

    reply->deleteLater();
}

void SearchMedia::endSelectMedia()
{
    qDebug() << getIdSelectMedia ();
    qDebug() << getSelectType ();
    qDebug() << "Close end search and select media!";
    this->sendRequestTMDBGetInformation();
}

void SearchMedia::on_okButton_clicked()
{
    this->close ();
    emit selectMedia();
}
