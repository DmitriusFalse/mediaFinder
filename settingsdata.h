#ifndef SETTINGSDATA_H
#define SETTINGSDATA_H
#include "LibraryItem.h"
#include "dbmanager.h"
#include <QMetaType>
#include <QString>
#include <QJsonObject>



class SettingsData
{
    // Q_OBJECT
public:
    SettingsData(DBManager *dbmanager);
    ~SettingsData();
    void writeLibraryToSettings(const QList<libraryItem> &data);
    QList<libraryItem> readLibraryFromSettings();
    QList<libraryItem> readLibraryFromSettings(QString type);
    QList<libraryItem> checkLibraryDuplicate(QList<libraryItem> libFolder);
    // QStringList videoExtensions = ;
    static const QString INSTALL_PATH;
    QString getInstallPath();

    QStringList getVideoExtensions() const;
private:
    DBManager* m_dbmanager;
};
#endif // SETTINGSDATA_H
