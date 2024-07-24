#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "Settings.h"
#include "dialogrenamerfiles.h"
#include "dialogshowprogress.h"
#include "searchmedia.h"
#include "settingsapp.h"
#include "medialibrary.h"
#include "dbmanager.h"

struct Connection {
    bool isConnected;
    QMetaObject::Connection connection;

    Connection() : isConnected(false) {}
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:

  void clickTreeWidgetMovie();
  void clickTreeWidgetTV();
  void updateCollections(QString type);
  void updateCollectionsByID(QString type, int id);
  void setLayoutVisibility(QLayout *layout, bool visible);
  void fillTvShowForm(int id);
  void fillTvShowEpisodeForm(int id, int seasonsNumber, int episodeNumber);
  void reloadSettings();

  Ui::MainWindow *ui;
  DBManager *dbmanager = nullptr;
  MediaLibrary *mediaLibrary = nullptr;
  SettingsData *settingsData = nullptr;
  SettingsApp *dialogSettingsApp = nullptr;
  SearchMedia *searchMedia = nullptr;
  DialogShowProgress *progressBar = nullptr;
  Settings *settings;
  DialogRenamerFiles *renameFiles = nullptr;

  QString NameShowLoaded="";

  Connection renameConnection;
      int numSeason, numEpisode, idShow, episodeID;

private slots:
    void onDialogClosed();
    void on_refreshLibrary_clicked();
    void on_openSettings_clicked();
    void slotChangetSelection();
    void on_loadMediaButton_clicked();
    void on_renameButton_clicked();
    void on_exitButton_clicked();
    void on_seasonEdit_textEdited(const QString &arg1);
    void on_saveSeasonEpisodeNum_clicked();
    void on_episodeEdit_textEdited(const QString &arg1);

public slots:
    // void slotUptateProgressBar(const int &i);
    void slotUptateProgressBar(const QString &str);
    void slotHideProgressBar();
    void slotUpdateListLibrary(QString type);
    void slotUpdateListLibraryByID(QString type, int id);
    void slotUpdateListLibraries();
    void slotEndSearch();
};
#endif // MAINWINDOW_H
