#ifndef SETTINGSAPP_H
#define SETTINGSAPP_H
#include "settingsdata.h"
#include "dbmanager.h"
#include <QMainWindow>
#include <QMetaType>
#include <QVariant>
#include <QString>

namespace Ui {
class SettingsApp;
}
class SettingsApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsApp(QWidget *parent, DBManager *dbManager, SettingsData *settings);
    ~SettingsApp();

private slots:
    void on_saveButton_clicked();
    void on_addPath_clicked();
    void on_applySaveSettings_clicked();
    void on_removeLibraryRow_clicked();

private:
    Ui::SettingsApp *ui;
    void addPathToListLibrary();
    void saveLibraryFolder(bool update);
    bool changeSettings;
    SettingsData *m_settings;
    DBManager *m_dbManager;
signals:
    void signalUpdateListCollection();
protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // SETTINGSAPP_H
