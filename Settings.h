#ifndef SETTINGS_H
#define SETTINGS_H
#include <QMap>
#include <QString>

struct Settings {
private:
    struct setting{
        QString name;
        QString value;
    };
    //QMap {Name seeting}, {seting}
    QMap<QString, setting> settingsMap;
public:
    QString getSettings(QString name){
        if (settingsMap.contains(name)) {
            return settingsMap[name].value;
        } else {
            return QString();
        }
    }
    void addSettings(QString name, QString value){
        settingsMap[name]=setting{name,value};
    }
    void addSettings(setting sett){
        settingsMap[sett.name]=sett;
    }
};

#endif // SETTINGS_H
