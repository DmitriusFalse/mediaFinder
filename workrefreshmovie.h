#ifndef WORKREFRESHMOVIE_H
#define WORKREFRESHMOVIE_H

#include "settingsdata.h"
#include <QThread>

class WorkRefreshMovie : public QThread
{
    Q_OBJECT
public:
    explicit WorkRefreshMovie(QObject *parent = nullptr);
    ~WorkRefreshMovie();
    void setPath(const QString &newPath);
    int getAction() const;
    void setActionScanMovie(); // 0
    void setActionScanTV(); // 1
    void setActionRecordDBMovie(); // 2
    void setActionRecordDBTV(); // 3
    void clearPathList();
private:
    void setAction(int newAction);
    QStringList scanFolders(QString path);
    SettingsData *m_settingsData;
    QThread workerThread;
    QStringList pathList;
    int action;
public slots:
    void startWork(const QString &path);
signals:
    void progressValueChanged(int value);
    void taskFinished();
    void taskScanFolderFinished(const QStringList &result);
    void progressUpdated(QString progress);
protected:
    void run() override;

};

#endif // WORKREFRESHMOVIE_H
