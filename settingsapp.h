#ifndef SETTINGSAPP_H
#define SETTINGSAPP_H

#include <QMainWindow>
#include <QMetaType>
#include <QVariant>
#include <QString>

namespace Ui {
class SettingsApp;
}
struct libraryFolder {
    QString path;
    QString type;
};
// Q_DECLARE_METATYPE(libraryFolder)
class SettingsApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsApp(QWidget *parent = nullptr);
    ~SettingsApp();

private slots:
    void on_saveButton_clicked();
    void on_addPath_clicked();

    void on_pushButton_clicked();

    void on_applySaveSettings_clicked();

private:
    Ui::SettingsApp *ui;
    void addPathToListLibrary();
    void writeStructToSettings(const QList<libraryFolder> &data);
    QList<libraryFolder> readStructFromSettings();
    QList<libraryFolder> checkDuplicate(QList<libraryFolder> libFolder);
    void saveLibraryFolder();
};

#endif // SETTINGSAPP_H
