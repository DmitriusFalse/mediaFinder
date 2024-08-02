#ifndef SETTINGSAPP_H
#define SETTINGSAPP_H
#include "dialogshowprogress.h"
#include "qnetworkreply.h"
#include "settingsdata.h"
#include "dbmanager.h"
#include <QMainWindow>
#include <QMetaType>
#include <QVariant>
#include <QString>
#include <QTranslator>

namespace Ui {
class SettingsApp;
}
struct requestGenre
{
public:
    bool enMovie = false;
    bool ruMovie = false;
    bool enShowTV = false;
    bool ruShowTV = false;

    bool checkRequest() const {
        return enMovie || ruMovie || enShowTV || ruShowTV;
    }
    void setRequest(const QString& language, const QString& name, bool flag) {
        if (language == "en" && name == "movie") {
            enMovie = flag;
        } else if (language == "ru" && name == "movie") {
            ruMovie = flag;
        } else if (language == "en" && name == "show") {
            enShowTV = flag;
        } else if (language == "ru" && name == "show") {
            ruShowTV = flag;
        }
    }
};
class SettingsApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsApp(QWidget *parent, DBManager *dbManager, SettingsData *settings, DialogShowProgress *sp);
    ~SettingsApp();

public slots:

private slots:
    void on_saveButton_clicked();
    void on_addPath_clicked();

    void on_removeLibraryRow_clicked();

    void on_refreshGenreList_clicked();
    void slotFinishRequestGetGenre(QNetworkReply *reply, QString lang, QString show);

    void on_pushButton_clicked();

private:
    Ui::SettingsApp *ui;
    void addPathToListLibrary();
    void saveLibraryFolder(bool update);
    void saveLangApp();
    bool changeSettings;
    void loadTranslation();
    void refreshGenresList();
    QTranslator translator;
    SettingsData *m_settings;
    DBManager *m_dbManager;
    DialogShowProgress *showProgres;
    requestGenre sendReuqestGenre;
signals:
    void signalUpdateListCollection();
    void signalWindowClosed();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // SETTINGSAPP_H
