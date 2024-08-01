#ifndef SEARCHMEDIA_H
#define SEARCHMEDIA_H

#include "GenreList.h"
#include "dialogshowprogress.h"
#include "medialibrary.h"
#include "dbmanager.h"
#include "qnetworkreply.h"
#include <QMainWindow>
#include "settingsdata.h"


namespace Ui {
class SearchMedia;
}

class SearchMedia : public QMainWindow
{
    Q_OBJECT

public:
    explicit SearchMedia(QWidget *parent, MediaLibrary *mLib, DBManager *db, DialogShowProgress *sp, Settings *param, SettingsData *setin);
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

    QString getLangSearch() const;
    void setLangSearch(const QString &newLangSearch);

    bool getTranslated() const;
    void setTranslated(bool newGetTranslated);

private:
    Ui::SearchMedia *uiSearch;
    QString nameSearch;
    QString typeMediaSearch;
    GenreList *genres;
    Settings *parametrs;
    void closeEvent(QCloseEvent *event) override;

    int idSelectMedia;
    int idTVDB;
    int idMovieDB;

    short countSendRequest;
    QString selectType;
    QString langSearch;
    bool translated;

    // QNetworkAccessManager *managerNetwork;
    MediaLibrary *mediaLibrary;
    DBManager *dbManager;
    ShowInfo *showTv;
    ShowInfo oldNameShow;
    MovieInfo *movie;
    MovieInfo oldMovie;
    DialogShowProgress *showProgres;
    SettingsData *settings;

    void setLangSearchMedia();

    void sendRequestTMDBSearch(QString Name, QString type);
    void sendRequestTMDBSearchGetImage();
    void sendRequestTMDBGetInformation(QString lang="en-EN");
    void sendRequestTMDBGetInformationEpisodes(QString lang="en-EN");
    void sendRequestTMDBGetImage();

    void getImageMovie();
    void getImageTVShow();

    QString updateField(QString oldString, QString newString);

    int progresSearchValue;
    void progresSearchAdd();
    void progresSearchShow();
signals:
    void windowClosed();
    void selectMedia();
    void endSearch();
private slots:

    void on_searchButton_clicked();
    void slotViewOverviewMedia();
    void slotFinishRequestFindMedia(QNetworkReply *reply, QString media_type);
    void slotUpdateImagesInTree(QNetworkReply *reply, int index);
    void slotChangetSelection();
    void slotFinishRequestChooseMedia(QNetworkReply *reply);
    void slotFinishRequestChooseMediaEpisodes(QNetworkReply *reply);
    void slotSavePosterFile(QNetworkReply *reply, QString pathFile);

    void processResponseTV(QJsonObject jsonObject);
    void processResponseMovie(QJsonObject jsonObject);
    void endSelectMedia();

    void on_selectFindMediaButton_clicked();
};

#endif // SEARCHMEDIA_H
