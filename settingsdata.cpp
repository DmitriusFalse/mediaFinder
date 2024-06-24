#include "settingsdata.h"
#include <QVariant>
#include <QDataStream>
#include <QSettings>
#include <QMetaType>
#include <QString>
Q_DECLARE_METATYPE(libraryItem);


const QString SettingsData::INSTALL_PATH = "/opt/MediaFinder";
SettingsData::SettingsData() {
    qRegisterMetaType<libraryItem>("libraryItem");
// static const QString INSTALL_PATH = "/opt/MediaFinder"
    // videoExtensions
}

void SettingsData::writeLibraryToSettings(const QList<libraryItem> &data)
{
    QSettings settings("MediaFinder", "settings");
    if(data.size()>0){
        settings.beginWriteArray("library", data.size());
        for (int i = 0; i < data.size(); ++i) {
            if (data.at(i).path != "" and data.at(i).type != ""){
                settings.setArrayIndex(i);
                settings.setValue("libraryItem"+ QString::number(i), QVariant::fromValue(data.at(i)));
            }
        }
        settings.endArray();
    }else{
        settings.remove("library");
    }
}

QList<libraryItem> SettingsData::readLibraryFromSettings()
{
    QSettings settings("MediaFinder", "settings");

    int size = settings.beginReadArray("library");
    QList<libraryItem> data;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QVariant variant = settings.value("libraryItem"+ QString::number(i));
        if (variant.canConvert<libraryItem>()) {
            libraryItem libFolder = variant.value<libraryItem>();
            data.append(libFolder);
        }
    }
    settings.endArray();

    return data;
}

QList<libraryItem> SettingsData::readLibraryFromSettings(QString type)
{
    QSettings settings("MediaFinder", "settings");

    int size = settings.beginReadArray("library");
    QList<libraryItem> data;
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QVariant variant = settings.value("libraryItem"+ QString::number(i));
        if (variant.canConvert<libraryItem>()) {
            libraryItem libFolder = variant.value<libraryItem>();
            if(libFolder.type==type){
                data.append(libFolder);
            }
        }
    }
    settings.endArray();

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
