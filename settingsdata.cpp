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
}

SettingsData::~SettingsData()
{
    delete m_dbmanager;
}

void SettingsData::writeLibraryToSettings(const QList<libraryItem> &data)
{
    this->m_dbmanager->truncateTable("Library");
    if(data.size()>0){
        for (auto& item : data) {
            this->m_dbmanager->writeLibrary(item.path, item.type);
        }
    }
}

QList<libraryItem> SettingsData::readLibraryFromSettings()
{
    // QSettings settings("MediaFinder", "settings");

    // int size = settings.beginReadArray("library");
    QList<libraryItem> data={};
    // for (int i = 0; i < size; ++i) {
    //     settings.setArrayIndex(i);
    //     QVariant variant = settings.value("libraryItem"+ QString::number(i));
    //     if (variant.canConvert<libraryItem>()) {
    //         libraryItem libFolder = variant.value<libraryItem>();
    //         data.append(libFolder);
    //     }
    // }
    // settings.endArray();
    // DBManager ms_dbmanager = *new DBManager();

    QStringList srcData = this->m_dbmanager->readLibrary();
    for (auto& str : srcData) {
        QStringList split = str.split(":");
        data << libraryItem{split[0], split[1]};

    }
    return data;
}

QList<libraryItem> SettingsData::readLibraryFromSettings(QString type)
{
    // QSettings settings("MediaFinder", "settings");

    // int size = settings.beginReadArray("library");
    QList<libraryItem> data={};
    // for (int i = 0; i < size; ++i) {
    //     settings.setArrayIndex(i);
    //     QVariant variant = settings.value("libraryItem"+ QString::number(i));
    //     if (variant.canConvert<libraryItem>()) {
    //         libraryItem libFolder = variant.value<libraryItem>();
    //         if(libFolder.type==type){
    //             data.append(libFolder);
    //         }
    //     }
    // }
    // settings.endArray();

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

QString SettingsData::getInstallPath()
{
    return INSTALL_PATH;
}
