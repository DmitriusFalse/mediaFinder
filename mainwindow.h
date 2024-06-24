#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "settingsapp.h"
#include "medialibrary.h"

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
    void on_pushButton_clicked();
    void onDialogClosed();
    void on_refreshLibrary_clicked();

private:
    Ui::MainWindow *ui;
    SettingsApp *dialog;
    SettingsData *settingsData;
    MediaLibrary *mediaLibrary;
    void onTreeWidgetItemSelected();
};
#endif // MAINWINDOW_H
