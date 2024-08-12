#ifndef SECRETVAULT_H
#define SECRETVAULT_H
#include <QString>
#include <QHash>

class Vault{
public:
    Vault();
    void putVault(const QString& key, const QString& value);
    void putVault(QHash<QString, QString> data);
    QString getValue(const QString& key);

private:
    QHash<QString, QString> hash;
};

#endif // SECRETVAULT_H
