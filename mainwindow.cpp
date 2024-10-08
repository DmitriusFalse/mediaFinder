#include "mainwindow.h"
#include <QTextDocument>
#include "settingsapp.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QTextBlock>
#include <QGraphicsBlurEffect>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QStyleFactory>
#include <QTranslator>
#include <QWebEngineView>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->dbmanager = new DBManager(this);
    // Получаем АПИ ключи доступа и назначаем его

    this->settingsData = new SettingsData(this->dbmanager);
    this->mediaLibrary = new MediaLibrary(this, this->dbmanager, settingsData);
    this->progressBar = new DialogShowProgress;
    this->dialogSettingsApp = new SettingsApp(this,dbmanager, this->settingsData, this->progressBar);
    this->showImageFile = new ShowImageFile;

    this->player = new videoPlayer;



    this->reloadSettings();
    this->loadTranslation();

    qApp->installTranslator(&translator);
    ui->retranslateUi(this);
    ui->PBRefreshLibrary->hide();

    ui->listMovieLibrary->setHeaderLabels({tr("Афиша"),tr("Название")});
    ui->listMovieLibrary->setColumnWidth(0,128);

    ui->listTVLibrary->setHeaderLabels({tr("Афиша"),tr("Название")});
    ui->listTVLibrary->setColumnWidth(0,128);

    QScrollBar *scrollMovieList = ui->listMovieLibrary->verticalScrollBar ();
    scrollMovieList->setStyleSheet("width: 20px;");

    QScrollBar *scrolShowTVList = ui->listTVLibrary->verticalScrollBar ();
    scrolShowTVList->setStyleSheet("width: 20px;");

    //Соединяем клик в QTreeWidget listMovieLibrary с функцией onTreeWidgetItemSelected
    connect(ui->listMovieLibrary, &QTreeWidget::itemSelectionChanged, this, &MainWindow::clickTreeWidgetMovie);
    connect(ui->listTVLibrary, &QTreeWidget::itemSelectionChanged, this, &MainWindow::clickTreeWidgetTV);

    //Отработка снятия выделения в Списке сериалов
    connect(ui->listTVLibrary, &QTreeWidget::itemSelectionChanged, this, &MainWindow::slotChangetSelection);
    //Отработка снятия выделения в Списке фильмов
    connect(ui->listMovieLibrary, &QTreeWidget::itemSelectionChanged, this, &MainWindow::slotChangetSelection);

    connect(this->mediaLibrary,
            &MediaLibrary::updateProgressBarUI,
            this,
            &MainWindow::slotUptateProgressBar);
    connect(this->dbmanager,
            &DBManager::signalUpdateProgresBar,
            this,
            &MainWindow::slotUptateProgressBar);
    connect(this->dbmanager,
            &DBManager::signalUpdateMainWindow,
            this,
            &MainWindow::slotUpdateListLibrary);

    connect(this->dbmanager,
            &DBManager::signalUpdateMainWindowByID,
            this,
            &MainWindow::slotUpdateListLibraryByID);

    connect(this->progressBar, &DialogShowProgress::emitCloseProgres, this, &MainWindow::onDialogClosed);
    //
    connect(this->dialogSettingsApp, &SettingsApp::signalWindowClosed, this, &MainWindow::onDialogClosed);

    playerConnection.connection = connect(this->player, &videoPlayer::closePlayer, this, &MainWindow::closePlayer);

    ui->listMovieLibrary->setAlternatingRowColors(true);
    ui->listTVLibrary->setAlternatingRowColors(true);
    this->updateCollections("Movie");
    this->updateCollections("TV");

    this->setLayoutVisibility(ui->mainDetailsLayout, false);
    this->setLayoutVisibility(ui->wrapTVShowLayout, false);
    ui->TVShowInfoTab->setCurrentIndex(0);
    ui->TVShowInfoTab->setTabEnabled(1, false);



}

MainWindow::~MainWindow()
{
    delete ui;
    delete this->dialogSettingsApp;
    // delete this->searchMedia;
    delete this->mediaLibrary;
    delete this->settingsData;
    delete this->dbmanager;

}

void MainWindow::onDialogClosed()
{
    this->reloadSettings();

    this->loadTranslation();
    // Включаем главное окно после закрытия диалогового окна
    setDisabled(false);
}

void MainWindow::on_refreshLibrary_clicked()
{
    int index = ui->tabMainWindow->currentIndex ();
    switch (index) {
    case 0:{
        qDebug() << tr("Обновляем список Фильмов");
        ui->PBRefreshLibrary->show ();
        ui->PBRefreshLibrary->setFormat (tr("Обновление коллекции фильмов!"));
        mediaLibrary->refsreshCollectionMovie();
        break;
    }
    case 1: {
        qDebug() << tr("Обновляем список Сериалов");
        mediaLibrary->refsreshCollectionTV ();
        break;
    }
    default:
        break;
    }

}

void MainWindow::clickTreeWidgetMovie()
{
    // click movie
    this->setLayoutVisibility(ui->mainDetailsLayout, true);
    QString apitoken = this->settingsData->getApiAccessToken("tmdbApiToken");
    if(apitoken.size()>0){
        ui->loadMediaButton->setDisabled(false);
    }else{
        ui->loadMediaButton->setDisabled(true);
    }
    ui->renameButton->setDisabled(false);
    QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();

    int id = selectedItem->data(0, Qt::UserRole).toInt();

    MovieInfo movie = dbmanager->getMovieByID(id);
    ui->fileMovieInfo->setText(movie.path);
    QPixmap posterPixmap(movie.poster);

    if(QFile::exists (movie.poster)){
        ui->zoomMovieImage->show();
        ui->zoomMovieImage->setProperty("image", QVariant::fromValue(movie.poster));
        ui->zoomMovieImage->setProperty("title", QVariant::fromValue(movie.name));
        posterPixmap.load (movie.poster);
    }else{
        posterPixmap.load (":/images/poster");
        ui->zoomMovieImage->hide();
    }

    QPixmap scaledPixmap = posterPixmap.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->posterLabel->setPixmap(scaledPixmap);
    ui->posterLabel->setWindowTitle(movie.name);
    if(movie.name.size()>0){
        ui->NameMovieEdit->setText(movie.name);
    }else{
        ui->NameMovieEdit->setText("-");
    }
    if(movie.overview.size()>0){
        ui->overviewEdit->setPlainText(movie.overview);
    }
    if(movie.imdbID != ""){
        ui->imdbText->setText(movie.imdbID);
    }else{
        ui->imdbText->setText("-");
    }
    if(movie.IDMovie>0){
        ui->TMDBText->setText(QString::number(movie.IDMovie));
    }else{
        ui->TMDBText->setText("-");
    }
    if(movie.Status.size()>0){
        ui->statusText->setText(movie.Status);
    }else{
        ui->statusText->setText("-");
    }
    ui->genreText->setText(movie.genre);

    QWidget *containerWidget = getWrapReviews(movie.reviews);
    ui->scrollAreaMovieReviews->setWidget(containerWidget);


    if(movie.videos.size()>0){
        ui->showVideoMovie->show();
        player->setVideoPlayList(movie.videos);
    }else{
        ui->showVideoMovie->hide();
    }
}

void MainWindow::clickTreeWidgetTV()
{
    // click show tv
    ui->saveSeasonEpisodeNum->hide();

    QTreeWidgetItem *selectedItem = ui->listTVLibrary->currentItem();

    QTreeWidgetItem *parentItem = selectedItem->parent(); // Получение родительского элемента
    this->setLayoutVisibility(ui->wrapTVShowLayout, true);

    if (parentItem != nullptr) { //Дочерний элемент
        //Episode select
        ui->TVShowInfoTab->setTabEnabled(1, true);
        ui->TVShowInfoTab->setCurrentIndex(1);
        this->numSeason = selectedItem->data(1, Qt::UserRole).toInt();
        this->numEpisode = selectedItem->data(2, Qt::UserRole).toInt();
        this->idShow = parentItem->data(0, Qt::UserRole).toInt();

        this->fillTvShowEpisodeForm(this->idShow, this->numSeason, this->numEpisode);

    } else { // Родительский элемент
        for (int i = 0; i < ui->listTVLibrary->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = ui->listTVLibrary->topLevelItem(i);
            if(item->parent()==nullptr){
                item->setExpanded(false);
            }
        }
        if(selectedItem->parent()==nullptr){
            selectedItem->setExpanded(true);
        }
        ui->TVShowInfoTab->setCurrentIndex(0);
        ui->TVShowInfoTab->setTabEnabled(1, false);

        int srcID = selectedItem->data(0, Qt::UserRole).toInt();

        this->fillTvShowForm(srcID);
    }


    QString apitoken = this->settingsData->getApiAccessToken("tmdbApiToken");
    if(apitoken.size()>0){
        ui->loadMediaButton->setDisabled(false);
    }else{
        ui->loadMediaButton->setDisabled(true);
    }
    ui->renameButton->setDisabled(false);

}

void MainWindow::updateCollections(QString type)
{

    QStringList action = {"Movie", "TV"};
    switch (action.indexOf(type)) {
        case 0:{
            MovieCollections movies= dbmanager->getMovieCollection ();
            ui->listMovieLibrary->clear();

            for (auto& movie : movies.movies) {

                QTreeWidgetItem *item = new QTreeWidgetItem(ui->listMovieLibrary);

                QPixmap pixmap;

                if(QFile::exists (movie.poster)){
                    pixmap.load (movie.poster);
                }else{
                    pixmap.load (":/images/poster");
                }

                QLabel *imageLabel = new QLabel();
                imageLabel->setPixmap(pixmap.scaled(90, 128));

                ui->listMovieLibrary->setItemWidget(item, 0, imageLabel);

                item->setText(1, movie.name);
                item->setData(0, Qt::UserRole, movie.id);

            }
            break;
        }
        case 1:{
            ui->listTVLibrary->clear ();
            TVCollection tvcols = dbmanager->getTVCollection ();
            for (const auto& show : tvcols.Show) {
                QTreeWidgetItem *mainItem = new QTreeWidgetItem(ui->listTVLibrary);
                mainItem->setText(1, show.nameShow);

                QPixmap pixmap;
                if(QFile::exists (show.poster)){
                    pixmap.load (show.poster);
                }else{
                    pixmap.load (":/images/poster");
                }
                QLabel *imageLabel = new QLabel();
                imageLabel->setPixmap(pixmap.scaled(90, 128));
                ui->listTVLibrary->setItemWidget(mainItem, 0, imageLabel);
                mainItem->setData(0, Qt::UserRole, show.ID);

                foreach (const uint seasonNumber, show.Episodes.keys()) {
                    const QMap<uint, EpisodeInfo>& episodes = show.Episodes[seasonNumber];
                    foreach (const uint episodeNumber, episodes.keys()) {
                        const EpisodeInfo& episodeInfo = episodes[episodeNumber];
                        QTreeWidgetItem *subItem1 = new QTreeWidgetItem(mainItem);
                        subItem1->setText(0, "S" + QString::number(episodeInfo.seasonsNumber) + "E" + QString::number(episodeInfo.episodeNumber));
                        subItem1->setText(1, episodeInfo.episodeTitle);
                        subItem1->setData(1,Qt::UserRole,episodeInfo.seasonsNumber);
                        subItem1->setData(2,Qt::UserRole,episodeInfo.episodeNumber);
                    }
                }

            }
            break;
        }
    }
}

void MainWindow::updateCollectionsByID(QString type, int id)
{
    QStringList action = {"Movie", "TV"};
    switch (action.indexOf(type)) {
        case 0:{ // movie

            QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();
            MovieInfo movie = dbmanager->getMovieByID(id);
            QPixmap pixmap;
            if(QFile::exists (movie.poster)){
                pixmap.load (movie.poster);
            }else{
                pixmap.load (":/images/poster");
            }
            QLabel *imageLabel = new QLabel();
            imageLabel->setPixmap(pixmap.scaled(90, 128));
            ui->listMovieLibrary->setItemWidget(selectedItem, 0, imageLabel);
            selectedItem->setText(1,movie.name);


            break;
            }
        case 1:{ //tv
                for (int index = 0; index < ui->listTVLibrary->topLevelItemCount(); ++index) {
                    QTreeWidgetItem *item = ui->listTVLibrary->topLevelItem(index);
                    int ID = item->data(0, Qt::UserRole).toInt();

                    if (id == ID) {
                        // Удаление существующих дочерних элементов
                        QList<QTreeWidgetItem*> children = item->takeChildren();
                        for (QTreeWidgetItem *child : children) {
                            delete child;
                        }

                        // Получение информации о шоу из базы данных
                        ShowInfo show = dbmanager->getShowTVShowByID(id);

                        // Загрузка постера шоу
                        QPixmap pixmap;
                        if (QFile::exists(show.poster)) {
                            pixmap.load(show.poster);
                        } else {
                            pixmap.load(":/images/poster");
                        }

                        // Создание QLabel для отображения постера и установка его в QTreeWidgetItem
                        QLabel *imageLabel = new QLabel();
                        imageLabel->setPixmap(pixmap.scaled(90, 128));
                        ui->listTVLibrary->setItemWidget(item, 0, imageLabel);

                        // Добавление информации о сезонах и эпизодах
                        for (const uint seasonNumber : show.Episodes.keys()) {
                            const QMap<uint, EpisodeInfo>& episodes = show.Episodes[seasonNumber];
                            for (const uint episodeNumber : episodes.keys()) {
                                const EpisodeInfo& episodeInfo = episodes[episodeNumber];
                                QTreeWidgetItem *subItem = new QTreeWidgetItem(item);
                                subItem->setText(0, "S" + QString::number(episodeInfo.seasonsNumber) + "E" + QString::number(episodeInfo.episodeNumber));
                                subItem->setText(1, episodeInfo.episodeTitle);
                                subItem->setData(1,Qt::UserRole,episodeInfo.seasonsNumber);
                                subItem->setData(2,Qt::UserRole,episodeInfo.episodeNumber);
                            }
                        }
                        // Прерывание цикла после обновления нужного элемента
                        break;
                    }
                }
            break;
            }
    }
}

void MainWindow::setLayoutVisibility(QLayout *layout, bool visible)
{
    if (!layout) return;

    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem *item = layout->itemAt(i);

        // Если элемент является виджетом
        if (QWidgetItem *widgetItem = dynamic_cast<QWidgetItem *>(item)) {
            QWidget *widget = widgetItem->widget();
            if (widget) {
                if (visible) {
                    widget->show();
                } else {
                    widget->hide();
                }
            }
        }

        // Если элемент является другой компоновкой
        if (QLayout *childLayout = item->layout()) {
            setLayoutVisibility(childLayout, visible);
        }
    }
}

void MainWindow::fillTvShowForm(int id)
{
    ShowInfo show = dbmanager->getShowTVShowByID(id);
    this->NameShowLoaded = show.nameShow;
    QPixmap posterPixmap(show.poster);

    if(QFile::exists (show.poster) && show.poster!=":/images/poster"){
        posterPixmap.load (show.poster);
        ui->zoomShowTVImage->show();
        ui->zoomShowTVImage->setProperty("image", QVariant::fromValue(show.poster));
        ui->zoomShowTVImage->setProperty("title", QVariant::fromValue(show.nameShow));
    }else{
        posterPixmap.load (":/images/poster");
        ui->zoomShowTVImage->hide();
    }

    QPixmap scaledPixmap = posterPixmap.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->posterTVShowLabel->setPixmap(scaledPixmap);
    ui->posterTVShowLabel->setWindowTitle(show.nameShow);
    if(show.nameShow.size()>0){
        ui->nameTVShow->setText(show.nameShow);
    }else{
        ui->nameTVShow->setText("-");
    }
    if(show.originalNameShow.size()>0){
        ui->originalNameShowText->setText(show.originalNameShow);
    }else{
        ui->originalNameShowText->setText("-");
    }
    if(show.overview.size()>0){
        ui->overviewTVShowText->setText(show.overview);
    }else{
        ui->overviewTVShowText->setText("-");
    }
    // ui->overviewTVShowText->setWordWrapMode(true);
    // ui->overviewTVShowText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if(show.idShow>0){
        ui->TMDBTVshowIDText->setText(QString::number(show.idShow));
    }else{
        ui->TMDBTVshowIDText->setText("-");
    }
    if(show.status.size()>0){
        ui->statusTVShowText->setText(show.status);
    }else{
        ui->statusTVShowText->setText("-");
    }

    QWidget *containerReviews = this->getWrapReviews(show.reviews);
    ui->scrollAreaShowTVReviews->setWidget(containerReviews);
    if(show.videos.size()>0){
        ui->showVideoTV->show();
        player->setVideoPlayList(show.videos);
    }else{
        ui->showVideoTV->hide();
    }


    ui->genreTVListLabel->setText(show.genres);

    if(show.numberOfSeasons>0){
        ui->infoCountSText->setText(QString::number(show.numberOfSeasons));
    }

    if(show.numberOfEpisodes>0){
        ui->infoCountEText->setText(QString::number(show.numberOfEpisodes));
    }
}

void MainWindow::fillTvShowEpisodeForm(int id, int seasonsNumber, int episodeNumber)
{
    ShowInfo showTV = dbmanager->getShowTVShowByID(id);
    if(this->NameShowLoaded!=showTV.nameShow){
        this->fillTvShowForm(id);
    }
    EpisodeInfo episode = showTV.getEpisode(seasonsNumber, episodeNumber);

    ui->fileEpisodeInfo->setText(episode.filePath);
    this->episodeID = episode.ID;
    if(episode.episodeTitle.size()>0){
        ui->nameEpisodeText->setText(episode.episodeTitle);
    }else{
        ui->nameEpisodeText->setText("-");
    }
    if(episode.overview.size()>0){
        ui->overviewEpisodeText->setText(episode.overview);
    }else{
        ui->overviewEpisodeText->setText("-");
    }
    // airDateText TMDBIdEpisodeText
    if(episode.air_date!=""){
        ui->airDateText->setText(episode.air_date);
    }else{
        ui->airDateText->setText("-");
    }
    if(episode.episodeID){
        ui->TMDBIdEpisodeText->setText(QString::number(episode.episodeID));
    }else{
        ui->TMDBIdEpisodeText->setText("-");
    }
    if(episode.seasonsNumber){
        ui->seasonEdit->setText(QString::number(episode.seasonsNumber));
    }else{
        ui->seasonEdit->setText("-");
    }
    if(episode.episodeNumber){
        ui->episodeEdit->setText(QString::number(episode.episodeNumber));
    }else{
        ui->episodeEdit->setText("-");
    }
    QPixmap posterPixmap;
    qDebug() << episode.still_path;
    if(QFile::exists (episode.still_path)){
        posterPixmap.load (episode.still_path);
        ui->zoomEpisodeImage->show();
        ui->zoomEpisodeImage->setProperty("image", QVariant::fromValue(episode.still_path));
        ui->zoomEpisodeImage->setProperty("title", QVariant::fromValue(showTV.nameShow+"-"+episode.episodeTitle+"S"+QString::number(episode.seasonsNumber)+"E"+QString::number(episode.episodeNumber)));
    }else{
        posterPixmap.load (":/images/poster");
        ui->zoomEpisodeImage->hide();
    }

    QPixmap scaledPixmap = posterPixmap.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->posterEpisode->setPixmap(scaledPixmap);
    ui->posterEpisode->setWindowTitle(episode.episodeTitle);
}

void MainWindow::reloadSettings()
{
    // Освобождаем память от старых настроек
    // Получаем новые настройки из базы данных
    this->settingsData->reloadVault();
    this->settingsData->reloadSettings();
}

void MainWindow::loadTranslation()
{
    if(!this->translator.load(":/translation/"+this->settingsData->getLangApp())){
        qDebug() << tr("Ошибка загрузки перевода: ") << this->settingsData->getLangApp();
    };

    qApp->installTranslator(&translator);
    ui->retranslateUi(this);
}

QWidget *MainWindow::getWrapReviews(QList<Reviews> reviews)
{
    QVBoxLayout *reviewDynamicLayout = new QVBoxLayout();
    QWidget *containerWidget = new QWidget();
    containerWidget->setLayout(reviewDynamicLayout);
    for (const Reviews& review : reviews) {

        QLabel *titleReviewLabel = new QLabel(review.author);
        QLabel *contentReviewLabel = new QLabel(review.content);

        QFrame *separated = new QFrame();
        separated->setFrameShape(QFrame::HLine);

        contentReviewLabel->setWordWrap(true);
        contentReviewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        reviewDynamicLayout->addWidget(titleReviewLabel);
        reviewDynamicLayout->addWidget(contentReviewLabel);
        reviewDynamicLayout->addWidget(separated);
        reviewDynamicLayout->addWidget(separated);
    }
    return containerWidget;
}

QWidget *MainWindow::getWrapVideo(QList<Videos> videos)
{
    QVBoxLayout *videoDynamicLayout = new QVBoxLayout();
    QWidget *containerWidget = new QWidget();
    containerWidget->setLayout(videoDynamicLayout);
    for (const Videos& video : videos) {
        // QtWebView *view = new QtWebView(this);
        // view->setUrl(QUrl("https://www.youtube.com/embed/"+video.key));
        // view->setMinimumHeight(480);
        // videoDynamicLayout->addWidget(view);
    }
    return containerWidget;
}


void MainWindow::slotUptateProgressBar(const QString &str)
{
    ui->PBRefreshLibrary->setFormat(str);
}

void MainWindow::slotHideProgressBar()
{
    ui->PBRefreshLibrary->hide ();
}

void MainWindow::slotUpdateListLibrary(QString type)
{
    ui->PBRefreshLibrary->hide ();
    MainWindow::updateCollections(type);
}

void MainWindow::slotUpdateListLibraryByID(QString type, int id)
{
    ui->PBRefreshLibrary->hide ();
    MainWindow::updateCollectionsByID(type, id);
}

void MainWindow::slotUpdateListLibraries()
{
    this->on_refreshLibrary_clicked();
    this->updateCollections("Movie");
    this->updateCollections("TV");
    this->reloadSettings();
}

void MainWindow::slotEndSearch()
{

    switch (ui->tabMainWindow->currentIndex ()) {
    case 0:{ //Movie
        this->updateCollections("Movie");
        }
        break;
    case 1:{ //TV
            this->updateCollections("TV");
        }
        break;
    }

}

void MainWindow::closePlayer()
{
    QList<Videos> videoList = player->getVideoPlayList();
    if(this->player!=nullptr){ // Проверяем существование инициализированного DialogRenamerFiles
        disconnect(playerConnection.connection);
        delete this->player;
    }

    this->player = new videoPlayer;
    this->player->setVideoPlayList(videoList);
    playerConnection.connection = connect(this->player, &videoPlayer::closePlayer, this, &MainWindow::closePlayer);
}

void MainWindow::slotChangetSelection()
{
    qDebug() << tr("Выделение снято");
    auto treeWidget = qobject_cast<QTreeWidget*>(sender());
    if (treeWidget->selectedItems().isEmpty()) {
        if(treeWidget->objectName()=="listMovieLibrary"){
            this->setLayoutVisibility(ui->mainDetailsLayout, false);
        }else if(treeWidget->objectName()=="listTVLibrary"){
            this->setLayoutVisibility(ui->wrapTVShowLayout, false);
        }

        ui->renameButton->setDisabled(true);
        // Код, который нужно выполнить при снятии выделения
        ui->loadMediaButton->setDisabled(true);
    }
}
void MainWindow::on_openSettings_clicked()
{
    this->setDisabled(true);
    this->dialogSettingsApp->setDisabled(false);


    connect (this->dialogSettingsApp,
            &SettingsApp::signalUpdateListCollection,
            this,
            &MainWindow::slotUpdateListLibraries);

    dialogSettingsApp->setWindowTitle(tr("Настройки - MediaFinder"));
    // Показываем диалоговое окно при нажатии на кнопку
    dialogSettingsApp->show();
}

void MainWindow::on_loadMediaButton_clicked()
{
    //Открытие окна поиска
    this->searchMedia = new SearchMedia(this, this->mediaLibrary,this->dbmanager,progressBar, this->settingsData);
    connect(this->searchMedia, &SearchMedia::windowClosed, this, &MainWindow::onDialogClosed);
    connect(this->searchMedia, &SearchMedia::endSearch, this,  &MainWindow::slotEndSearch);
    MainWindow::setDisabled (true);
    this->searchMedia->setDisabled (false);
    // this->searchWindow->setAttribute(Qt::WA_DeleteOnClose);
    this->searchMedia->setWindowTitle(tr("Обновление метаданных - MediaFinder"));
    QTreeWidgetItem *selectedItem;
    switch (ui->tabMainWindow->currentIndex ()) {
    case 0:{ //Movie
        selectedItem = ui->listMovieLibrary->currentItem();
        int idMediaDB = selectedItem->data(0, Qt::UserRole).toInt();
        this->searchMedia->setIdMovieDB (idMediaDB);
        this->searchMedia->setSearchWord (selectedItem->text(1));
        this->searchMedia->fillFields ("Movie");
        break;
    }
    case 1:{ //TV
        selectedItem = ui->listTVLibrary->currentItem(); // Получение текущего выделенного элемента
        if (selectedItem != nullptr) {
            QTreeWidgetItem *parentItem = selectedItem->parent(); // Получение родительского элемента
            int idMediaDB = 0;
            QString searchText="";
            if (parentItem != nullptr) {
                idMediaDB = parentItem->data(0, Qt::UserRole).toInt();
                searchText = parentItem->text(1);
            } else {
                idMediaDB = selectedItem->data(0, Qt::UserRole).toInt();
                searchText = selectedItem->text(1);
            }
            this->searchMedia->setIdTVDB (idMediaDB);
            this->searchMedia->setSearchWord (searchText);
            this->searchMedia->fillFields ("TV");
        }
        break;
    }
    }


    this->searchMedia->show();
}


void MainWindow::on_renameButton_clicked()
{
    if(this->renameFiles!=nullptr){ // Проверяем существование инициализированного DialogRenamerFiles
        // отключаем соединения
        if(renameConnection.isConnected){
            disconnect(renameConnection.connection);
        }
        // и удаляем
        delete this->renameFiles;
    }

    this->renameFiles = new DialogRenamerFiles(this, dbmanager);

    // 0 - Movie
    // 1 - Show TV
    switch (ui->tabMainWindow->currentIndex()) {
    case 0:{
        QTreeWidgetItem *selectedItem;
        selectedItem = ui->listMovieLibrary->currentItem();
        int idMediaDB = selectedItem->data(0, Qt::UserRole).toInt();
        MovieInfo movie = dbmanager->getMovieByID(idMediaDB);
        this->renameFiles->setMediaData(movie);
    }break;
    case 1:{
        QTreeWidgetItem *selectedItem;
        selectedItem = ui->listTVLibrary->currentItem();
        QTreeWidgetItem *parentItem = selectedItem->parent(); // Получение родительского элемента
        int idMediaDB = 0;
        QString searchText="";
        if (parentItem != nullptr) {
            idMediaDB = parentItem->data(0, Qt::UserRole).toInt();
            searchText = parentItem->text(1);
        } else {
            idMediaDB = selectedItem->data(0, Qt::UserRole).toInt();
            searchText = selectedItem->text(1);
        }
        ShowInfo show = dbmanager->getShowTVShowByID(idMediaDB);
        this->renameFiles->setMediaData(show);
    }break;
    }
    renameConnection.connection = connect(this->renameFiles,
            &DialogRenamerFiles::signalFinishRename,
            this,
            &MainWindow::slotUpdateListLibraryByID);
    renameConnection.isConnected = true;
    this->renameFiles->setTypeMedia(ui->tabMainWindow->currentIndex());
    this->renameFiles->show();
}


void MainWindow::on_exitButton_clicked()
{
    this->close();
}

void MainWindow::on_seasonEdit_textEdited(const QString &arg1)
{
    ui->saveSeasonEpisodeNum->show();
    this->numSeason = arg1.toInt();
}

void MainWindow::on_episodeEdit_textEdited(const QString &arg1)
{
    ui->saveSeasonEpisodeNum->show();
    this->numEpisode = arg1.toInt();
}


void MainWindow::on_saveSeasonEpisodeNum_clicked()
{
    if(this->episodeID>0){
        if(this->numSeason>=0){
            this->dbmanager->updateEpisodeColumn("Season", this->numSeason, this->episodeID);
        }
        if(this->numEpisode>=0){
            this->dbmanager->updateEpisodeColumn("Episode", this->numEpisode, this->episodeID);
        }

        QTreeWidgetItem *selectedItem = ui->listTVLibrary->currentItem();
        selectedItem->setText(0, "S" + QString::number(this->numSeason) + "E" + QString::number(this->numEpisode));

        selectedItem->setData(1,Qt::UserRole,this->numSeason);
        selectedItem->setData(2,Qt::UserRole,this->numEpisode);

        this->fillTvShowEpisodeForm(this->idShow, this->numSeason, this->numEpisode);
    }
    ui->saveSeasonEpisodeNum->hide();
}

void MainWindow::on_zoomEpisodeImage_clicked()
{
    QString image = ui->zoomEpisodeImage->property("image").toString();
    QString title = ui->zoomEpisodeImage->property("title").toString();
    this->showImageFile->setImage(image,title);
    this->showImageFile->show();
}

void MainWindow::on_tabMainWindow_currentChanged(int index)
{
    ui->renameButton->setEnabled(false);
    switch (index) {
        case 0:{
            ui->listTVLibrary->clearSelection();
        }
        break;
        case 1:{
            ui->listMovieLibrary->clearSelection();
        }
        break;
    }
}

void MainWindow::on_zoomShowTVImage_clicked()
{
    QString image = ui->zoomShowTVImage->property("image").toString();
    QString title = ui->zoomShowTVImage->property("title").toString();
    this->showImageFile->setImage(image,title);
    this->showImageFile->show();
}

void MainWindow::on_zoomMovieImage_clicked()
{
    QString image = ui->zoomMovieImage->property("image").toString();
    QString title = ui->zoomMovieImage->property("title").toString();
    this->showImageFile->setImage(image,title);
    this->showImageFile->show();
}


void MainWindow::on_filterLine_textChanged(const QString &world)
{
    QTreeWidget *currentTree = nullptr;
    switch (ui->tabMainWindow->currentIndex()) {
    case 0:{
        currentTree = ui->listMovieLibrary;
    }break;
    case 1:
        currentTree = ui->listTVLibrary;
        break;
    }
    if(currentTree!=nullptr){
        for (int i = 0; i < currentTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = currentTree->topLevelItem(i);
            QString name = item->text(1);
            if(!name.contains(world)){
                currentTree->topLevelItem(i)->setHidden(true);
            }else{
                currentTree->topLevelItem(i)->setHidden(false);
            }
        }
    }
}


void MainWindow::on_showVideoTV_clicked()
{


    player->open();
}


void MainWindow::on_showVideoMovie_clicked()
{


    player->open();
}

