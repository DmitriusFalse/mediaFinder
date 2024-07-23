#ifndef MOVIECOLLECTIONS_H
#define MOVIECOLLECTIONS_H
#include "qdebug.h"
#include <QString>

struct Reviews {
    QString nameShow;
    QString author;
    QString content;
    int idShow;
};
struct Videos {
    int idShow;
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
struct Crew {
    int id; //id
    QString name; //name
    QString role; //job
    QString thumb; //profile_path
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

    EpisodeInfo(): index(credits.constBegin()) {}
    // Метод добавления Crew по отдельным параметрам
    void addCrew(int id, const QString& name, const QString& role, const QString& thumb) {
        credits[id] = Crew{id, name, role, thumb};
        index = credits.constBegin();
    }

    // Метод добавления Crew по объекту
    void addCrew(const Crew& credit) {
        credits[credit.id] = credit;
        index = credits.constBegin();
    }
    Crew getCreditByID(int id){
        if(credits.contains(id)){
            return credits[id];
        }else{
            return Crew{};
        }
    }
    bool nextCrew(){
        // Если текущий итератор не достиг конца
        if (index != credits.constEnd()) {
            return true;
        }
        index = credits.constBegin(); // Сброс итератора
        return false;
    }
    Crew& getCrew(){
        Crew& returnValue = const_cast<Crew&>(index.value());
        index ++;
        return returnValue;
    }
private:
    QMap<int, Crew> credits;
    QMap<int, Crew>::const_iterator index;
};
struct ShowInfo {
    int ID;

    QString imdb_id;
    QString nameShow;
    QString originalNameShow;
    QString poster;
    QString status;
    QString genres;
    QString overview;
    QString libraryPath;
    QString production_companies;
    QString logoPath;
    QString first_air_date;
    QString last_air_date;

    int idShow;
    int numberOfSeasons; // количество сезонов
    int numberOfEpisodes; // количество серий всего

    QList<Reviews> reviews;

    QList<Videos> videos;

    QMap<int, Crew> credits;
    QMap<int, Crew>::const_iterator index;

    QMap<uint, QMap<uint,EpisodeInfo>> Episodes;
    void addEpisodes(EpisodeInfo episode){
        if (Episodes.contains(episode.seasonsNumber) && Episodes[episode.seasonsNumber].contains(episode.episodeNumber)) {
            EpisodeInfo& existingEpisode = Episodes[episode.seasonsNumber][episode.episodeNumber];

            // Обновление полей при наличии данных
            if (!episode.episodeTitle.isEmpty()) {
                existingEpisode.episodeTitle = episode.episodeTitle;
            }
            if (!episode.filePath.isEmpty()) {
                existingEpisode.filePath = episode.filePath;
            }
            if (!episode.air_date.isEmpty()) {
                existingEpisode.air_date = episode.air_date;
            }
            if (!episode.still_path.isEmpty()) {
                existingEpisode.still_path = episode.still_path;
            }
            if (!episode.overview.isEmpty()) {
                existingEpisode.overview = episode.overview;
            }
            if (!episode.pathToSerial.isEmpty()) {
                existingEpisode.pathToSerial = episode.pathToSerial;
            }
            if (!episode.libraryPath.isEmpty()) {
                existingEpisode.libraryPath = episode.libraryPath;
            }
            // ID предполагается уникальным и не изменяемым, поэтому не проверяем его.
        } else {
            // Добавление нового эпизода
            Episodes[episode.seasonsNumber][episode.episodeNumber] = episode;
        }
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
    void addVideos(QString key, int idShow){
        Videos video;
        video.idShow = idShow;
        video.key= key;
        this->addVideos(video);
    }
    void addCrew(int id, const QString& name, const QString& role, const QString& thumb) {
        credits[id] = Crew{id, name, role, thumb};
        index = credits.constBegin();
    }

    // Метод добавления Crew по объекту
    void addCrew(const Crew& credit) {
        credits[credit.id] = credit;
        index = credits.constBegin();
    }
    Crew getCreditByID(int id){
        if(credits.contains(id)){
            return credits[id];
        }else{
            return Crew{};
        }
    }
    bool nextCrew(){
        // Если текущий итератор не достиг конца
        if (index != credits.constEnd()) {
            return true;
        }
        index = credits.constBegin(); // Сброс итератора
        return false;
    }
    Crew& getCrew(){
        Crew& returnValue = const_cast<Crew&>(index.value());
        index ++;
        return returnValue;
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
