#ifndef DIALOGRENAMERFILES_H
#define DIALOGRENAMERFILES_H

#include "dbmanager.h"
#include "movieCollections.h"
#include "qtreewidget.h"
#include <QDialog>

namespace Ui {
class DialogRenamerFiles;
}

struct PlaceholdersMovie
{
    QMap<QString, QString> placeholders;
    QMap<QString, QString> fieldDescriptions; // Новый QMap для хранения описаний полей
    QStringList keysList; // Список ключей для удобства итерации
    int currentIndex=-1; // Текущий индекс для метода next()

    PlaceholdersMovie(const MovieInfo& movie) {
        updateData(movie);
    }

    void updateData(const MovieInfo& movie) {
        placeholders.clear(); // Очищаем текущие данные перед обновлением
        fieldDescriptions.clear(); // Очищаем описания полей перед обновлением
        keysList.clear(); // Очищаем список ключей

        if (!movie.name.isEmpty()) {
            placeholders[":name"] = movie.name;
            fieldDescriptions[":name"] = "Название фильма";
            keysList.append(":name");
        }
        if (!movie.originalName.isEmpty()) {
            placeholders[":originalName"] = movie.originalName;
            fieldDescriptions[":originalName"] = "Оригинальное название";
            keysList.append(":originalName");
        }
        if (movie.IDMovie != 0) {
            placeholders[":idMovie"] = QString::number(movie.IDMovie);
            fieldDescriptions[":idMovie"] = "ID фильма";
            keysList.append(":idMovie");
        }
        if (movie.imdbID != 0) {
            placeholders[":imdbID"] = QString::number(movie.imdbID);
            fieldDescriptions[":imdbID"] = "IMDB ID";
            keysList.append(":imdbID");
        }
        if (!movie.originalLang.isEmpty()) {
            placeholders[":originalLang"] = movie.originalLang;
            fieldDescriptions[":originalLang"] = "Оригинальный язык";
            keysList.append(":originalLang");
        }
        if (!movie.release_date.isEmpty()) {
            placeholders[":release_date"] = movie.release_date;
            fieldDescriptions[":release_date"] = "Дата релиза";
            keysList.append(":release_date");
        }
        if (!movie.Status.isEmpty()) {
            placeholders[":Status"] = movie.Status;
            fieldDescriptions[":Status"] = "Статус";
            keysList.append(":Status");
        }

        currentIndex = -1; // Инициализируем текущий индекс
    }

    QString getValue(const QString& key) const {
        return placeholders.value(key, QString());
    }

    QString getFieldDescription(const QString& key) const {
        return fieldDescriptions.value(key, QString());
    }

    QMap<QString, QString> getFilledFields() const {
        return placeholders;
    }

    bool next() {
        ++currentIndex;
        return currentIndex < keysList.size();
    }

    QString currentKey() const {
        if (currentIndex < keysList.size()) {
            return keysList[currentIndex];
        }
        return QString();
    }
};
struct PlaceholdersTV {
    QMap<uint, QMap<uint, QMap<QString, QString>>> data;
    QMap<QString, QString> fieldDescriptions;
    QList<QString> keys;
    int currentIndex = -1;

    PlaceholdersTV(ShowInfo &showInfo) {
        updateData(showInfo);
        initializeFieldDescriptions();
    }

    void updateData(ShowInfo &showInfo) {
        for (auto season : showInfo.Episodes.keys()) {
            for (auto episode : showInfo.Episodes[season].keys()) {
                EpisodeInfo episodeInfo = showInfo.getEpisode(season, episode);
                data[season][episode][":nameShow"] = showInfo.nameShow;
                data[season][episode][":originalNameShow"] = showInfo.originalNameShow;
                data[season][episode][":status"] = showInfo.status;
                data[season][episode][":idShow"] = QString::number(showInfo.idShow);
                data[season][episode][":episodeNumber"] = QString::number(episodeInfo.episodeNumber);
                data[season][episode][":seasonsNumber"] = QString::number(episodeInfo.seasonsNumber);
                data[season][episode][":episodeTitle"] = episodeInfo.episodeTitle;
                data[season][episode][":air_date"] = episodeInfo.air_date;
            }
        }
    }

    void initializeFieldDescriptions() {
        fieldDescriptions[":nameShow"] = "Название шоу";
        fieldDescriptions[":originalNameShow"] = "Оригинальное название шоу";
        fieldDescriptions[":status"] = "Статус шоу";
        fieldDescriptions[":idShow"] = "ID шоу";
        fieldDescriptions[":episodeNumber"] = "Номер серии";
        fieldDescriptions[":seasonsNumber"] = "Номер сезона";
        fieldDescriptions[":episodeTitle"] = "Название серии";
        fieldDescriptions[":air_date"] = "Дата выхода серии";
    }

    QString getValue(uint season, uint episode, const QString &key) const {
        if (data.contains(season) && data[season].contains(episode) && data[season][episode].contains(key)) {
            return data[season][episode][key];
        } else {
            return QString();
        }
    }
};
class DialogRenamerFiles : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRenamerFiles(QWidget *parent, DBManager *db);
    ~DialogRenamerFiles();
    void setTypeMedia(int index);
    void setMediaData(MovieInfo mov);
    void setMediaData(ShowInfo sh);

private slots:
    void on_patternMovieEdit_textChanged(const QString &arg1);

    void on_renameMovieButton_clicked();


    void on_patternTVEdit_textChanged(const QString &arg1);

    void on_renameTVButton_clicked();

    void on_folderSeasonsCheckBox_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::DialogRenamerFiles *ui;
    DBManager *dbmanager;
    MovieInfo movie;
    ShowInfo showTv;
    PlaceholdersMovie *replacePlaceholdersMovie;
    PlaceholdersTV *replacePlaceholdersTV;
    QString newFileName;
    QTreeWidgetItem *oldIitem;

    void changeNameTv(QString pattern);
    void changeNameMovie(QString pattern);
    QString replacePatternMovie(const QString& input);
    QString replacePatternTV(const QString& input, int Season, int Episode);
    QString renameFile(const QString& filePath, const QString& newName);
    QString renameAndMoveFile(const QString& oldPath, const QString& newPath);
    bool checkNewFoldersEpisodes;
signals:
    void signalFinishRename(QString type, int id);
};

#endif // DIALOGRENAMERFILES_H
