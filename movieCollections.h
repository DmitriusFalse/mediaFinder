#ifndef MOVIECOLLECTIONS_H
#define MOVIECOLLECTIONS_H
#include "qdebug.h"
#include <QString>

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

struct MovieInfo{
    int id;
    int IDMovie;
    int imdbID;
    QString genre;
    QString path;
    QString poster;
    QString originalName;
    QString name;
    QString originalLang;
    QString release_date;
    QString library_path;
    QString overview;
    QString production_companies;
    QString logoCompanies;
    QString Status;
    QList<Reviews> reviews;
    QList<Videos> videos;
    void addReviews(Reviews review){
        this->reviews.append(review);
    }
    void addVideos(Videos video){
        this->videos.append(video);
    }
    void addVideos(QList<Videos> video){
        this->videos.append(video);
    }
    void addVideos(QString key){
        this->addVideos(Videos{.key = key});
    }
};
struct MovieCollections {
    QList<MovieInfo> movies;
    void addMovie(const MovieInfo &item) {
        movies.append(item);
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
private:
public:
    QList<ShowInfo> Show;
    void addShow(ShowInfo newShow){
        this->Show.append(newShow);
    }
    uint size(){
        return this->Show.size();
    }
};
#endif // MOVIECOLLECTIONS_H
