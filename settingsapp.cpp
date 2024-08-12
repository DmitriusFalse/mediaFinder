#include "settingsapp.h"
#include "GenreList.h"
#include "qevent.h"
#include "qmessagebox.h"
#include "ui_settingsapp.h"
#include "settingsdata.h"
#include <QComboBox>
#include <QFileDialog>
#include <QVariant>
#include <QDataStream>
#include <QSettings>
#include <QMetaType>
#include <QString>
#include <QTranslator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include "qurlquery.h"
#include <QJsonArray>

//////////////////////////////////////////////////////////////////////////
/// \brief SettingsApp::SettingsApp
/// \param parent
//////////////////////////////////////////////////////////////////////////
SettingsApp::SettingsApp(QWidget *parent, DBManager *dbManager, SettingsData *settings, DialogShowProgress *sp)
    : QMainWindow(parent)
    , ui(new Ui::SettingsApp)
    , m_settings(settings)
    , m_dbManager(dbManager)
    , showProgres(sp)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    QString tmdbApiToken = this->m_settings->getApiAccessToken("tmdbApiToken");
    ui->apiKeyText->setPlainText(tmdbApiToken);

    SettingsApp::addPathToListLibrary();

    changeSettings = false;

    QString defLang = settings->getLangApp();

    if(defLang=="en-EN"){
        ui->defLangComboBox->setCurrentIndex(0);
    }else if(defLang=="ru-RU"){
        ui->defLangComboBox->setCurrentIndex(1);
    }else{
        ui->defLangComboBox->setCurrentIndex(0);
    }
    ui->tableDirsType->setColumnWidth(0, 400);
    ui->tableDirsType->setColumnWidth(1, 100);
    this->loadTranslation();
    this->refreshGenresList();



}

SettingsApp::~SettingsApp()
{

    delete ui;
}

void SettingsApp::saveLibraryFolder(bool update){
    QList<libraryItem> libFolders;
    for (int row = 0; row < ui->tableDirsType->rowCount(); ++row) {
        QTableWidgetItem *itemPath = ui->tableDirsType->item(row, 0);
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableDirsType->cellWidget(row, 1));
        libFolders << libraryItem {itemPath->text(), comboBox->currentText()};
    }
    m_settings->writeLibraryToSettings(libFolders);
    if(update){
        emit signalUpdateListCollection();
    }
}

void SettingsApp::saveLangApp()
{
//settings

    switch (ui->defLangComboBox->currentIndex()) {
    case 0:{
        this->m_settings->saveSettings("language",QVariant(QString{"en-EN"}));
    }
    break;
    case 1:{

        this->m_settings->saveSettings("language",QVariant(QString{"ru-RU"}));
    }
    break;
default:
    break;
}

}

void SettingsApp::loadTranslation()
{
    QString lang = m_settings->getLangApp();
    if(!this->translator.load(":/translation/"+lang)){
        qDebug() << tr("Ошибка загрузки перевода:");
    };
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);
}

void SettingsApp::refreshGenresList()
{

    ui->tableGenreList->setColumnCount(3); // Установите количество столбцов

    GenreList genre_ru = m_dbManager->loadGenre ("ru-RU");
    GenreList genre_en = m_dbManager->loadGenre ("en-EN");
    if(genre_ru.size()>genre_en.size()){
        ui->tableGenreList->setRowCount(genre_ru.size());
    }else{
        ui->tableGenreList->setRowCount(genre_en.size());
    }
    uint index = 0;
    showProgres->setMainLineMessage(tr("Выводим жанры"));
    showProgres->updateProgres();
    while(genre_ru.next()){
        showProgres->updateProgres();
        genre g = genre_ru.get();
        QTableWidgetItem *itemId = new QTableWidgetItem(QString::number(g.id));
        ui->tableGenreList->setItem(index, 0, itemId);
        QTableWidgetItem *itemRu = new QTableWidgetItem(g.name);
        ui->tableGenreList->setItem(index, 1, itemRu);
        index++;
    }

    index = 0;
    while(genre_en.next()){
        showProgres->updateProgres();
        genre g = genre_en.get();
        QTableWidgetItem *itemRu = new QTableWidgetItem(g.name);
        ui->tableGenreList->setItem(index, 2, itemRu);
        index++;
    }
    ui->tableGenreList->resizeColumnsToContents();
    showProgres->closeProgres();
}

void SettingsApp::closeEvent(QCloseEvent *event)
{
    if (changeSettings==false){
        event->accept();
    }else{
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Подтверждение"),
                                      tr("Сохранить изменения перед выходом?"),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            SettingsApp::saveLibraryFolder(true);
            event->accept(); // Разрешаем закрытие
        } else {
            event->accept(); // Отменяем закрытие
        }
    }
    emit signalWindowClosed();

}

void SettingsApp::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();  // Закрыть окно при нажатии клавиши Esc
    } else {
        QMainWindow::keyPressEvent(event);  // Передать событие базовому классу
    }
}

void SettingsApp::on_saveButton_clicked()
{
    // Сохранение настроек программы
    changeSettings = false;
    this->saveLangApp();
    this->m_settings->reloadSettings();
    this->loadTranslation();
    SettingsApp::saveLibraryFolder(true);

    QString tmdbApiToken = ui->apiKeyText->toPlainText();
    m_settings->saveApiKey("tmdbApiToken", tmdbApiToken);
    ui->apiKeyText->setPlainText(tmdbApiToken);
    this->close();
}

void SettingsApp::on_addPath_clicked()
{
    changeSettings = true;
    // qRegisterMetaType<libraryItem>("libraryItem");
    QList<libraryItem> libFolders = m_settings->readLibraryFromSettings();
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          tr("Выберите папку"),
                                                          QDir::homePath(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    bool yesAdd = true;
    for (auto& libFolder : libFolders) {

        if(libFolder.path.startsWith (directory+"/")){
            yesAdd = false;
        }
    }
    if (yesAdd) {
        libFolders << libraryItem{directory, "Movie"};
        libFolders = m_settings->checkLibraryDuplicate(libFolders);
        m_settings->writeLibraryToSettings(libFolders);

        SettingsApp::addPathToListLibrary();
    }else{
        changeSettings = false;
        QMessageBox::information(nullptr, tr("Информация"),
                                 tr("Путь к библиотеке не добавлен.\n"
                                 "Путь '/home/user/media' является родительским\n"
                                 "для уже добавленного пути.\n"
                                 "Пожалуйста, выберите другую папку."));
    }
}

void SettingsApp::addPathToListLibrary(){

    qRegisterMetaType<libraryItem>("libraryItem");

    QList<libraryItem> libraryFolders = m_settings->readLibraryFromSettings();

    int size = libraryFolders.size();

    SettingsApp::ui->tableDirsType->setRowCount(size);
    SettingsApp::ui->tableDirsType->setColumnCount(2);
    SettingsApp::ui->tableDirsType->setHorizontalHeaderLabels(QStringList() << tr("Библиотека") << tr("Тип"));

    int row = 0; // номер строки, куда вы хотите вставить выпадающий список
    int column = 1; // номер столбца, куда вы хотите вставить выпадающий список
    int iconSize = 24;

    for (const auto &libFolder : libraryFolders) {
        QTableWidgetItem *itemPath = new QTableWidgetItem(libFolder.path);
        itemPath->setFlags(itemPath->flags() & ~Qt::ItemIsEditable);
        ui->tableDirsType->setItem(row, 0, itemPath);
        QComboBox *comboBox = new QComboBox();
        comboBox->setIconSize(QSize(iconSize, iconSize));
        QIcon iconMovie(":/icons/movie");
        QPixmap pixmapMovie = iconMovie.pixmap(iconSize, iconSize);

        comboBox->addItem(QIcon(pixmapMovie),"Movie");

        QIcon iconTV(":/icons/tv");
        QPixmap pixmapTV = iconTV.pixmap(iconSize, iconSize);
        comboBox->addItem(QIcon(pixmapTV),"TV");
        comboBox->setCurrentText(libFolder.type);
        SettingsApp::ui->tableDirsType->setCellWidget(row, column, comboBox);
        row++;
    }
    ui->tableDirsType->resizeColumnsToContents();

}

void SettingsApp::on_removeLibraryRow_clicked()
{
    changeSettings = true;
    int row = ui->tableDirsType->currentRow();
    if (row >= 0) {
        ui->tableDirsType->removeRow(row);
    }
}

void SettingsApp::on_refreshGenreList_clicked()
{
    qDebug() << tr("Загружаем Жанры");
    QByteArray api = "Bearer " + m_settings->getApiAccessToken("tmdbApiToken");
    showProgres->reset();
    showProgres->show();
    showProgres->setMainLineMessage("Обновляем жанры");
    showProgres->setTextProgres("Английский - Фильмы");
    showProgres->updateProgres();

    QString lang ="en";
    QString name = "movie";

    QString urlStringEnMovie = "https://api.themoviedb.org/3/genre/movie/list";
    QUrl urlEnMovie(urlStringEnMovie);
    QUrlQuery queryEnMovie;
    queryEnMovie.addQueryItem("language", "en");

    urlEnMovie.setQuery (queryEnMovie);
    QNetworkRequest requestEnMovie(urlEnMovie);

    // Установка заголовков

    requestEnMovie.setRawHeader("Authorization", api);
    requestEnMovie.setRawHeader("accept", "application/json");

    QNetworkAccessManager *managerEnMovie = new QNetworkAccessManager(this);

    connect(managerEnMovie, &QNetworkAccessManager::finished, this,
            [this, lang, name](QNetworkReply *reply) {
                this->slotFinishRequestGetGenre(reply, lang, name);
            });

    managerEnMovie->get(requestEnMovie);

    showProgres->updateProgres();
    sendReuqestGenre.enMovie = true;


    showProgres->setTextProgres("Русский - Фильмы");
    lang ="ru";
    QString urlStringRuMovie = "https://api.themoviedb.org/3/genre/movie/list";
    QUrl urlRuMovie(urlStringRuMovie);
    QUrlQuery queryRuMovie;
    queryRuMovie.addQueryItem("language", "ru");

    urlRuMovie.setQuery (queryRuMovie);
    QNetworkRequest requestRuMovie(urlRuMovie);

    // Установка заголовков

    requestRuMovie.setRawHeader("Authorization", api);
    requestRuMovie.setRawHeader("accept", "application/json");

    QNetworkAccessManager *managerRuMovie = new QNetworkAccessManager(this);

    connect(managerRuMovie, &QNetworkAccessManager::finished, this,
            [this, lang, name](QNetworkReply *reply) {
                this->slotFinishRequestGetGenre(reply, lang, name);
            });

    managerRuMovie->get(requestRuMovie);

    showProgres->updateProgres();
    sendReuqestGenre.ruMovie = true;


    showProgres->setTextProgres("Английский - Сериалы");
    lang ="en";
    name="show";
    QString urlStringEnShowTv = "https://api.themoviedb.org/3/genre/tv/list";
    QUrl urlEnShowTv(urlStringEnShowTv);
    QUrlQuery queryEnShowTv;
    queryEnShowTv.addQueryItem("language", "en");

    urlEnShowTv.setQuery (queryEnShowTv);
    QNetworkRequest requestEnShowTv(urlEnShowTv);

    // Установка заголовков
    requestEnShowTv.setRawHeader("Authorization", api);
    requestEnShowTv.setRawHeader("accept", "application/json");

    QNetworkAccessManager *managerEnShowTv = new QNetworkAccessManager(this);

    connect(managerEnShowTv, &QNetworkAccessManager::finished, this,
            [this, lang, name](QNetworkReply *reply) {
                this->slotFinishRequestGetGenre(reply, lang, name);
            });

    managerEnShowTv->get(requestEnShowTv);

    showProgres->updateProgres();
    sendReuqestGenre.enShowTV = true;

    showProgres->setTextProgres("Русский - Сериалы");
    lang = "ru";
    QString urlStringRuShowTv = "https://api.themoviedb.org/3/genre/tv/list";
    QUrl urlRuShowTv(urlStringRuShowTv);
    QUrlQuery queryRuShowTv;
    queryRuShowTv.addQueryItem("language", "ru");

    urlRuShowTv.setQuery (queryRuShowTv);
    QNetworkRequest requestRuShowTv(urlRuShowTv);

    // Установка заголовков
    requestRuShowTv.setRawHeader("Authorization", api);
    requestRuShowTv.setRawHeader("accept", "application/json");

    QNetworkAccessManager *managerRuShowTv = new QNetworkAccessManager(this);

    connect(managerRuShowTv, &QNetworkAccessManager::finished, this,
            [this, lang, name](QNetworkReply *reply) {
                this->slotFinishRequestGetGenre(reply, lang, name);
            });
            // &SettingsApp::slotFinishRequestGetGenre);

    managerRuShowTv->get(requestRuShowTv);

    showProgres->updateProgres();
    sendReuqestGenre.ruShowTV = true;

}

void SettingsApp::slotFinishRequestGetGenre(QNetworkReply *reply, QString lang, QString show)
{
    qDebug() << tr("Получаем ответ о Жанрах");
    showProgres->updateProgres();
    showProgres->setTextProgres(tr("Жанры получены"));
    GenreList genresList;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll(); // Читаем полученные данные
        QJsonObject jsonObject = QJsonDocument::fromJson(data).object ();
        QJsonArray genresArray = jsonObject.value ("genres").toArray ();
        for (const QJsonValue& genresValue : genresArray) {
            QJsonObject genresObject = genresValue.toObject();
            int id = genresObject.value("id").toInt();
            QString name = genresObject.value("name").toString();
            genresList.addGenre(id, name, lang);
            sendReuqestGenre.setRequest(lang, show,false);
        }
        m_dbManager->saveGenres(genresList);
    } else {
        qDebug() << tr("Ошибка выбора медиа - Серии:") << reply->errorString();
    }
    if(!sendReuqestGenre.checkRequest()){
        this->refreshGenresList();
    }
}

void SettingsApp::on_pushButton_clicked()
{
    this->close();
}

