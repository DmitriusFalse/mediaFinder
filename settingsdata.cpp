#include "settingsdata.h"
#include <QVariant>
#include <QDataStream>
#include <QSettings>
#include <QMetaType>
#include <QString>
#include "dbmanager.h"


Q_DECLARE_METATYPE(libraryItem);


const QString SettingsData::INSTALL_PATH = "/opt/MediaFinder";

SettingsData::SettingsData(DBManager *dbmanager) : m_dbmanager(dbmanager) {
    qRegisterMetaType<libraryItem>("libraryItem");
    this->vault = new Vault;
    this->reloadVault();
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

QStringList SettingsData::getVideoExtensions() const
{
    return {"mp4", "avi", "mkv", "mov", "wmv", "flv", "mpeg", "mpg"};
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

QString SettingsData::getInstallPath()
{
    return INSTALL_PATH;
}
