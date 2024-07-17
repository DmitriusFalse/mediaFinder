#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include "Settings.h"
#include "dialogshowprogress.h"
#include "searchmedia.h"
#include "settingsapp.h"
#include "medialibrary.h"
#include "dbmanager.h"

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
  DBManager *dbmanager;
  MediaLibrary *mediaLibrary;
  SettingsData *settingsData;
  SettingsApp *dialogSettingsApp;
  SearchMedia *searchMedia;
  DialogShowProgress *progressBar;
  Settings *settings;

  QString NameShowLoaded="";

private slots:
    void onDialogClosed();
    void on_refreshLibrary_clicked();
    void on_openSettings_clicked();
    void slotChangetSelection();
    void on_loadMediaButton_clicked();

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
