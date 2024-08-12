#include "settingsdata.h"
#include <QVariant>
#include <QDataStream>
#include <QSettings>
#include <QMetaType>
#include <QString>
#include "dbmanager.h"


Q_DECLARE_METATYPE(libraryItem);

SettingsData::SettingsData(DBManager *dbmanager) : m_dbmanager(dbmanager) {
    this->vault = new Vault;
    this->reloadVault();
    this->reloadSettings();


}

SettingsData::~SettingsData()
{
    delete this->vault;
}

void SettingsData::writeLibraryToSettings(const QList<libraryItem> &data)
{

    QList<libraryItem> oldLibraryList = this->readLibraryFromSettings ();

    for (auto& oldItem : oldLibraryList) {
        bool delOldItem = true;
        for (auto& item : data) {
            if (oldItem.path == item.path){
                delOldItem = false;
            }
        }
        if (delOldItem){
            m_dbmanager->removeMovieCollectionsLibrary(oldItem.path);
        }
    }

    this->m_dbmanager->truncateTable("Library");
    if(data.size()>0){
        for (auto& item : data) {
            this->m_dbmanager->writeLibrary(item.path, item.type);
        }
    }
}

QList<libraryItem> SettingsData::readLibraryFromSettings()
{
    QList<libraryItem> data={};
    QStringList srcData = this->m_dbmanager->readLibrary();
    for (auto& str : srcData) {
        QStringList split = str.split(":");
        data << libraryItem{split[0], split[1]};
    }
    return data;
}

QList<libraryItem> SettingsData::readLibraryFromSettings(QString type)
{
    QList<libraryItem> data={};
    QStringList srcData;
    srcData.append(this->m_dbmanager->readLibrary(type));
    for (auto& str : srcData) {
        QStringList split = str.split(":");
        data << libraryItem{split[0], split[1]};

    }
    return data;
}

QList<libraryItem> SettingsData::checkLibraryDuplicate(QList<libraryItem> libFolder)
{
    QList<libraryItem> newLibFolders;
    for (const auto &itemFolder : libFolder) {
        bool insert = true;
        for (const auto &newItemFolder : newLibFolders) {
            if(itemFolder.path==newItemFolder.path){
                insert = false;
                break;
            }
        }
        if (insert == true){
            newLibFolders << itemFolder;
        }
    }
    return newLibFolders;
}

void SettingsData::saveApiKey(const QString &name, const QString &api)
{
    this->m_dbmanager->putVault(name, api);
    this->vault->putVault(name, api);
}

QByteArray SettingsData::getApiAccessToken(const QString &name)
{
    return this->vault->getValue(name).toUtf8();
}

void SettingsData::reloadVault()
{
  QHash<QString, QString> hash = m_dbmanager->getVault();
    this->vault->putVault(hash);
}

QVariant SettingsData::getSettings(QString name){
    if (settingsMap.contains(name)) {
        return settingsMap[name].value;
    } else {
        return QVariant();
    }
}

void SettingsData::addSettings(QString name, QVariant value){
    settingsMap[name]=setting{name,value};
}

void SettingsData::addSettings(setting sett){
    settingsMap[sett.name]=sett;
}

void SettingsData::saveSettings(QString name, QVariant value)
{
    this->m_dbmanager->saveSettings(name, value);
    this->reloadSettings();
}

QString SettingsData::getLangApp()
{
    QVariant var = this->getSettings("language");
    if(!var.isNull()){
        // QStringList data = var.toString().split(":");
        return var.toString();
    }else{
        return "ru-RU";
    }
}

void SettingsData::reloadSettings()
{
    QMap<QString, QVariant> var = m_dbmanager->getAllSettings();

    for (const QString& key : var.keys()) {
        QVariant value = var.value(key);
        this->addSettings(key, value);
    }
    this->addSettings("videoExtendion",QVariant(QStringList{"mp4", "avi", "mkv", "mov", "wmv", "flv", "mpeg", "mpg"}));
}
