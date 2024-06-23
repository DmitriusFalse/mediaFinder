#ifndef SETTINGSDATA_H
#define SETTINGSDATA_H
#include <QString>
#include <QMetaType>


struct libraryItem {
    QString path;
    QString type;
};
class SettingsData
{
public:
    SettingsData();
    static void writeStructToSettings(const QList<libraryItem> &data);
    static QList<libraryItem> readStructFromSettings();
    static QList<libraryItem> checkDuplicate(QList<libraryItem> libFolder);
};

#endif // SETTINGSDATA_H
