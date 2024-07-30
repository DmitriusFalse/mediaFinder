#include "secretVault.h"
#include <QObject>

Vault::Vault(QObject *parent)
{

}
QHash<QString, QString> Vault::hash;
void Vault::putVault(const QString &key, const QString &value)
{
    hash[key] = value;
}

QString Vault::getValue(const QString &key)
{
    if(hash.contains(key)){
        return hash[key];
    }
    return QString();
}
