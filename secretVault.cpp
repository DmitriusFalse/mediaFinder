#include "secretVault.h"

Vault::Vault()
{

}

void Vault::putVault(const QString &key, const QString &value)
{
    hash[key] = value;
}

void Vault::putVault(QHash<QString, QString> data)
{
    this->hash = data;
}

QString Vault::getValue(const QString &key)
{
    if(hash.contains(key)){
        return hash[key];
    }
    return QString();
}
