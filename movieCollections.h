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
    int idMedia;
    QString key;
    QString name; //name
    QString getYouTubeEmbed(){
        return "https://www.youtube.com/embed/" + this->key;
    }
};

struct Crew {
    int id; //id
    QString name; //name
    // QString role; //job
    QString thumb; //profile_path
    QSet<QString> role;
    void addRole(QString data){
        role.insert(data);
    }
    bool hasRole(const QString& targetRole) const {
        return role.contains(targetRole);
    }
};
struct MovieInfo{
    int id;
    int IDMovie;
    QString imdbID;
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
    MovieInfo() : index(credits.constBegin()){}
    void addReviews(Reviews review){
        this->reviews.append(review);
    }
    void addVideos(Videos video){
        this->videos.append(video);
    }
    void addVideos(QList<Videos> video){
        this->videos.append(video);
    }
    // void addVideos(QString key){
    //     this->addVideos(Videos{.key = key});
    // }

    // Метод добавления Crew по объекту
    void addCrew(const Crew& credit) {
        if (credits.contains(credit.id)) {
            // Если Crew с таким ID уже существует, обновляем его
            Crew& existingCrew = credits[credit.id];
            existingCrew.role.unite(credit.role);  // Объединяем наборы ролей
        } else {
            // Если Crew с таким ID не существует, добавляем новый
            credits[credit.id] = credit;
        }
        index = credits.constBegin();  // Сбрасываем итератор
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
    int getSizeCredits(){
        return credits.size();
    }
    void resetIterator(){
        index = credits.constBegin();  // Сбрасываем итератор
    }
private:
    QMap<int, Crew> credits;
    QMap<int, Crew>::const_iterator index;
};
struct MovieCollections {
    QList<MovieInfo> movies;
    void addMovie(const MovieInfo &item) {
        movies.append(item);
    }
};

struct EpisodeInfo {
    int ID=0;
    int episodeNumber=0;
    int seasonsNumber=0;
    int episodeID=0;
    QString episodeTitle="";
    QString filePath="";
    QString air_date="";
    QString still_path="";
    QString overview="";
    QString pathToSerial="";
    QString libraryPath="";

    EpisodeInfo(): index(credits.constBegin()) {}
    // Метод добавления Crew по отдельным параметрам
    void addCrew(int id, const QString& name, const QString& role, const QString& thumb) {
        Crew existCrew;
        existCrew.name = name;
        existCrew.id = id;
        existCrew.addRole(role);
        existCrew.thumb = thumb;
        this->addCrew(existCrew);
    }
    // Метод добавления Crew по объекту
    void addCrew(const Crew& credit) {
        if (credits.contains(credit.id)) {
            // Если Crew с таким ID уже существует, обновляем его
            Crew& existingCrew = credits[credit.id];
            existingCrew.role.unite(credit.role);  // Объединяем наборы ролей
        } else {
            // Если Crew с таким ID не существует, добавляем новый
            credits[credit.id] = credit;
        }
        index = credits.constBegin();  // Сбрасываем итератор
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
    int getSizeCredits(){
        return credits.size();
    }
    void resetIterator(){
        index = credits.constBegin(); // Сброс итератора
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
    void addVideos(QString key, QString name, uint id){
        Videos video;
        video.idMedia = id;
        video.key= key;
        video.name = name;
        this->addVideos(video);
    }
    void addCrew(int id, const QString& name, const QString& role, const QString& thumb) {
        Crew existCrew;
        existCrew.name = name;
        existCrew.id = id;
        existCrew.addRole(role);
        existCrew.thumb = thumb;
        this->addCrew(existCrew);
    }
    // Метод добавления Crew по объекту
    void addCrew(const Crew& credit) {
        if (credits.contains(credit.id)) {
            // Если Crew с таким ID уже существует, обновляем его
            Crew& existingCrew = credits[credit.id];
            existingCrew.role.unite(credit.role);  // Объединяем наборы ролей
        } else {
            // Если Crew с таким ID не существует, добавляем новый
            credits[credit.id] = credit;
        }
        index = credits.constBegin();  // Сбрасываем итератор
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
    void resetIterator(){
        index = credits.constBegin();  // Сбрасываем итератор
    }
private:
    QMap<int, Crew> credits;
    QMap<int, Crew>::const_iterator index;
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
