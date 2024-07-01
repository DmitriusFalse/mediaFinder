#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private slots:
    void onDialogClosed();
    void on_refreshLibrary_clicked();
    void on_openSettings_clicked();

private:
    void clickTreeWidgetMovie();
    void clickTreeWidgetTV();
    void updateCollections(QString type);
    Ui::MainWindow *ui;
    SettingsApp *dialogSettingsApp;
    SettingsData *settingsData;
    MediaLibrary *mediaLibrary;
    DBManager *m_dbmanager;
public slots:
    // void slotUptateProgressBar(const int &i);
    void slotUptateProgressBar(const QString &str);
    void slotHideProgressBar();
    void slotUpdateListLibrary(QString type);
    void slotUpdateListLibraries();
};
#endif // MAINWINDOW_H
