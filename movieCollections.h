#ifndef MOVIECOLLECTIONS_H
#define MOVIECOLLECTIONS_H
#include "qdebug.h"
#include <QString>
struct movieItem{
    int id;
    QString genre;
    QString path;
    QString poster;
    QString name;
    QString library_path;
    QString description;
};
struct movieCollections {
    QList<movieItem> items;
    int size;
    movieCollections(QList<movieItem>& itemsList) : items(itemsList) {
        this->size=items.size ();
    }
    void append(const movieItem &item) {
        items.append(item);
        size++;
    }
    void remove(int index) {
        if (index >= 0 && index < items.size()) {
            items.removeAt(index); // Удаляем элемент по индексу
            size--;
        } else {
            qDebug() << "Ошибка: Неверный индекс для удаления элемента.";
        }
    }
};

struct EpisodeInfo {
    int ID;
    int episodeNumber;
    int seasonsNumber;
    QString episodeTitle;
    QString filePath;
    QString air_date;
    QString still_path;
    QString overview;
    QString pathToSerial;
    QString libraryPath;
};
struct Reviews {
    QString nameShow;
    QString author;
    QString content;
};
struct Videos {
    QString key;
    QString getYouTubeEmbed(){
        return "https://www.youtube.com/embed/" + this->key;
    }
};

struct ShowInfo {
    int ID;
    QString nameShow;
    QString originalNameShow;
    QString poster;
    QString status;
    QString genres;
    QString overview;
    QString libraryPath;
    QString production_companies;
    QString logoPath;
    int idShow;
    int numberOfSeasons; // количество сезонов
    int numberOfEpisodes; // количество серий всего
    QList<Reviews> reviews;
    QList<Videos> videos;
    QMap<uint, QMap<uint,EpisodeInfo>> Episodes;
    void addEpisodes(EpisodeInfo episode){
        this->Episodes[episode.seasonsNumber][episode.episodeNumber] = episode;
    }
    EpisodeInfo getEpisode(uint season, uint episode) {
        return Episodes[season][episode];
    }
    void addReviews(Reviews review){
        this->reviews.append(review);
    }
    void addVideos(Videos video){
        this->videos.append(video);
    }
    void addVideos(QString key){
        this->addVideos(Videos{.key = key});
    }
};

struct TVCollection {
    QList<ShowInfo> Show;
};
#endif // MOVIECOLLECTIONS_H
