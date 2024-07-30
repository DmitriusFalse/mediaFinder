#ifndef SECRETVAULT_H
#define SECRETVAULT_H
#include <QObject>
#include <QString>
#include <QHash>

class Vault : public QObject {
    Q_OBJECT
public:
    Vault(QObject *parent = nullptr);
    static void putVault(const QString& key, const QString& value);
    static QString getValue(const QString& key);

private:
    static QHash<QString, QString> hash;
};

#endif // SECRETVAULT_H
