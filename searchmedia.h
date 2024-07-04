#ifndef SEARCHMEDIA_H
#define SEARCHMEDIA_H

#include "medialibrary.h"
#include "dbmanager.h"
#include <QMainWindow>


struct GenreList   {
public:

    struct genre {
        int id;
        QString name;
    };
    QString getGenre(int id){
        auto it = genreMap.find(id);
        if (it != genreMap.end()) {
            return it.value().name;
        } else {
            qDebug() << "Жанр с ID" << id << "не найден.";
            return QString::number (id);
        }
    }
    GenreList& addGenre(int id, const QString& name) {
        genreMap.insert(id, genre{id, name});
        return *this;
    }
private:
    QHash<uint, genre> genreMap;
};

namespace Ui {
class SearchMedia;
}

class SearchMedia : public QMainWindow
{
    Q_OBJECT

public:
    explicit SearchMedia(QWidget *parent, MediaLibrary *mLib, DBManager *db);
    ~SearchMedia();
    void fillFields(QString type);
    void setSearchWord(const QString &newNameSearch);
    void setTypeMediaSearch(const QString &newTypeMediaSearch);

private:
    Ui::SearchMedia *uiSearch;
    QString nameSearch;
    QString typeMediaSearch;
    GenreList *genres;
    void closeEvent(QCloseEvent *event) override;


    MediaLibrary *mediaLibrary;
    DBManager *dbManager;

    void sendRequestTMDBSearch(QString Name, QString type);
    void sendRequestTMDBGetImage();
signals:
    void windowClosed();
private slots:
    void on_searchButton_clicked();
    void slotViewOverviewMedia();
    void slotFinishRequestFindMedia(QNetworkReply *reply, QString media_type);
    void slotUpdateImagesInTree(QNetworkReply *reply, int index);
};

#endif // SEARCHMEDIA_H
