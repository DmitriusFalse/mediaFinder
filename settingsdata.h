#ifndef SETTINGSDATA_H
#define SETTINGSDATA_H

#include "LibraryItem.h"
#include "dbmanager.h"
#include <QMetaType>
#include <QString>
#include <QJsonObject>
#include "secretVault.h"
#include <QObject>
struct setting{
    QString name;
    QVariant value;
};
class SettingsData : QObject
{
    Q_OBJECT
private:
    DBManager* m_dbmanager;

    QMap<QString, setting> settingsMap;
public:
    SettingsData(DBManager *dbmanager);
    ~SettingsData();
    void writeLibraryToSettings(const QList<libraryItem> &data);
    QList<libraryItem> readLibraryFromSettings();
    QList<libraryItem> readLibraryFromSettings(QString type);
    QList<libraryItem> checkLibraryDuplicate(QList<libraryItem> libFolder);

    void saveApiKey(const QString &name, const QString& api);
    QByteArray getApiAccessToken(const QString &name);

    Vault* vault;
    void reloadVault();

    QVariant getSettings(QString name);
    void addSettings(QString name, QVariant value);
    void addSettings(setting sett);
    void saveSettings(QString name, QVariant value);
    QString getLangApp();
    void reloadSettings();

};
#endif // SETTINGSDATA_H
