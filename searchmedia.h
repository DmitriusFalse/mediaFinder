#ifndef SEARCHMEDIA_H
#define SEARCHMEDIA_H

#include "GenreList.h"
#include "medialibrary.h"
#include "dbmanager.h"
#include "qnetworkreply.h"
#include <QMainWindow>



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

    int getIdSelectMedia() const;
    void setIdSelectMedia(int newIdSelectMedia);

    QString getSelectType() const;
    void setSelectType(const QString &newSelectType);

    int getIdTVDB() const;
    void setIdTVDB(int newIdTVDB);

    int getIdMovieDB() const;
    void setIdMovieDB(int newIdMovieDB);

private:
    Ui::SearchMedia *uiSearch;
    QString nameSearch;
    QString typeMediaSearch;
    GenreList *genres;
    void closeEvent(QCloseEvent *event) override;

    int idSelectMedia;
    int idTVDB;
    int idMovieDB;

    short countSendRequest;
    QString selectType;


    MediaLibrary *mediaLibrary;
    DBManager *dbManager;
    ShowInfo *showTv;
    void sendRequestTMDBSearch(QString Name, QString type);
    void sendRequestTMDBGetImage();
    void sendRequestTMDBGetInformation();
    void sendRequestTMDBGetInformationEpisodes(int count);
signals:
    void windowClosed();
    void selectMedia();
private slots:

    void on_searchButton_clicked();
    void slotViewOverviewMedia();
    void slotFinishRequestFindMedia(QNetworkReply *reply, QString media_type);
    void slotUpdateImagesInTree(QNetworkReply *reply, int index);
    void slotChangetSelection();
    void slotFinishRequestChooseMedia(QNetworkReply *reply);
    void slotFinishRequestChooseMediaEpisodes(QNetworkReply *reply);
    void endSelectMedia();
    void on_okButton_clicked();

};

#endif // SEARCHMEDIA_H
