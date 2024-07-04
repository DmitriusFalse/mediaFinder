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

    uiSearch->viewSearchTree->setColumnWidth(0,128);
    uiSearch->viewSearchTree->setColumnWidth(2,96);
    uiSearch->viewSearchTree->setColumnWidth(3,128);

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

void SearchMedia::sendRequestTMDBSearch(QString Name, QString type)
{
    qDebug() << "Ищем фильмы... сериалы...";
    QUrl url(QString("https://api.themoviedb.org/3/search/multi"));

    QUrlQuery query;
    query.addQueryItem("query", Name);
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


void SearchMedia::sendRequestTMDBGetImage()
{
    qDebug() << "Получаем картинки";


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
        this->sendRequestTMDBGetImage();

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
