#ifndef DIALOGRENAMERFILES_H
#define DIALOGRENAMERFILES_H

#include "dbmanager.h"
#include "movieCollections.h"
#include "qtreewidget.h"
#include <QDialog>

namespace Ui {
class DialogRenamerFiles;
}

struct Placeholders
{
    QMap<QString, QString> placeholders;
    QMap<QString, QString> fieldDescriptions; // Новый QMap для хранения описаний полей
    QStringList keysList; // Список ключей для удобства итерации
    int currentIndex=-1; // Текущий индекс для метода next()

    Placeholders(const MovieInfo& movie) {
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

// QString name
// QString originalName
// int idMovie
// int imdbID
// QString originalLang
// QString release_date
// QString Status


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


private:
    Ui::DialogRenamerFiles *ui;
    DBManager *dbmanager;
    MovieInfo movie;
    Placeholders *replacePlaceholders;
    QString newFileName;
    QTreeWidgetItem *oldIitem;

    void changeName(QString pattern);
    QString replacePattern(const QString& input);
    QString renameFile(const QString& filePath, const QString& newName);
signals:
    void signalFinishRename(QString type, int id);
};

#endif // DIALOGRENAMERFILES_H
