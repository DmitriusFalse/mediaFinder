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


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{

    qRegisterMetaType<libraryItem>("libraryItem");
    qRegisterMetaType<MovieCollections>("movieCollections");
    qRegisterMetaType<MovieInfo>("movieItem");
    qRegisterMetaType<TVCollection>("TVCollection");
    qRegisterMetaType<ShowInfo>("SerialInfo");
    // qRegisterMetaType<SeasonInfo>("SeasonInfo");
    qRegisterMetaType<EpisodeInfo>("EpisodeInfo");
    ui->setupUi(this);
    this->dbmanager = new DBManager(this);
    this->settings = dbmanager->getAllSettings();
    this->settingsData = new SettingsData(this->dbmanager);
    this->mediaLibrary = new MediaLibrary(this, this->dbmanager, settingsData);
    this->progressBar = new DialogShowProgress;
    this->dialogSettingsApp = new SettingsApp(this,dbmanager, settingsData, this->settings);



    ui->PBRefreshLibrary->hide();
    ui->listMovieLibrary->setStyleSheet("QTreeWidget::item { height: 128px; }");
    ui->listMovieLibrary->setHeaderLabels({"Афиша","Название"});
    ui->listMovieLibrary->setColumnWidth(0,128);

    ui->listTVLibrary->setHeaderLabels({"Афиша","Название"});
    ui->listTVLibrary->setColumnWidth(0,128);

    QScrollBar *verticalScrollBar = ui->listMovieLibrary->verticalScrollBar ();
    verticalScrollBar->setStyleSheet("width: 30px;");



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

    ui->listMovieLibrary->setAlternatingRowColors(true);
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
    // Включаем главное окно после закрытия диалогового окна
    setDisabled(false);
}


void MainWindow::on_refreshLibrary_clicked()
{
    int index = ui->tabMainWindow->currentIndex ();
    switch (index) {
    case 0:{
        qDebug() << "Refresh Movie";
        ui->PBRefreshLibrary->show ();
        ui->PBRefreshLibrary->setFormat ("Обновление коллекции фильмов!");
        mediaLibrary->refsreshCollectionMovie();
        break;
    }
    case 1: {
        qDebug() << "Refresh TV";
        mediaLibrary->refsreshCollectionTV ();
        break;
    }
    default:
        break;
    }

}

void MainWindow::clickTreeWidgetMovie()
{

    qDebug() << "Click Movie";
    this->setLayoutVisibility(ui->mainDetailsLayout, true);
    ui->loadMediaButton->setDisabled(false);
    ui->renameButton->setDisabled(false);
    QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();

    int id = selectedItem->data(0, Qt::UserRole).toInt();

    MovieInfo movie = dbmanager->getMovieByID(id);

    QPixmap posterPixmap(movie.poster);

    if(QFile::exists (movie.poster)){
        posterPixmap.load (movie.poster);
    }else{
        posterPixmap.load ("/opt/MediaFinder/poster.png");
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
    if(movie.imdbID>0){
        ui->imdbText->setText(QString::number(movie.imdbID));
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
    // ui->reviewsLayout;
    QVBoxLayout *reviewDynamicLayout = new QVBoxLayout();
    QWidget *containerWidget = new QWidget();
    containerWidget->setLayout(reviewDynamicLayout);
    for (const Reviews& review : movie.reviews) {

        QLabel *titleReviewLabel = new QLabel(review.author);
        QLabel *contentReviewLabel = new QLabel(review.content);

        QFrame *separated = new QFrame();
        separated->setFrameShape(QFrame::HLine);

        contentReviewLabel->setWordWrap(true);
        contentReviewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        // contentReviewLabel->setFixedHeight(textHeight);
        // QLabel *contentReviewLabel = new QLabel(review.content);
        reviewDynamicLayout->addWidget(titleReviewLabel);
        reviewDynamicLayout->addWidget(contentReviewLabel);
        reviewDynamicLayout->addWidget(separated);
        reviewDynamicLayout->addWidget(separated);
    }
    ui->scrollArea->setWidget(containerWidget);
    // ui->reviewsContentLayout->addLayout(reviewDynamicLayout);
}

void MainWindow::clickTreeWidgetTV()
{
    qDebug() << "Click TV";

    QTreeWidgetItem *selectedItem = ui->listTVLibrary->currentItem();
    QTreeWidgetItem *parentItem = selectedItem->parent(); // Получение родительского элемента
    this->setLayoutVisibility(ui->wrapTVShowLayout, true);

    if (parentItem != nullptr) {
        //Episode select
        qDebug() << "1";
        ui->TVShowInfoTab->setTabEnabled(1, true);
        ui->TVShowInfoTab->setCurrentIndex(1);
        int seasonsNumber = selectedItem->data(1, Qt::UserRole).toInt();
        int episodeNumber = selectedItem->data(2, Qt::UserRole).toInt();
        int srcID = parentItem->data(0, Qt::UserRole).toInt();

        this->fillTvShowEpisodeForm(srcID, seasonsNumber, episodeNumber);

    } else {
        ui->TVShowInfoTab->setCurrentIndex(0);
        ui->TVShowInfoTab->setTabEnabled(1, false);

        int srcID = selectedItem->data(0, Qt::UserRole).toInt();

        this->fillTvShowForm(srcID);
    }


    ui->loadMediaButton->setDisabled(false);
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
                    pixmap.load ("/opt/MediaFinder/poster.png");
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
                    pixmap.load ("/opt/MediaFinder/poster.png");
                }
                QLabel *imageLabel = new QLabel();
                imageLabel->setPixmap(pixmap.scaled(90, 128));
                ui->listTVLibrary->setItemWidget(mainItem, 0, imageLabel);
                qDebug() << show.ID;
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
    qDebug() << "type " << type << " id " << id;
    switch (action.indexOf(type)) {
        case 0:{ // movie

            QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();
            int srcID = selectedItem->data(0, Qt::UserRole).toInt();
            MovieInfo movie = dbmanager->getMovieByID(id);
            QPixmap pixmap;
            if(QFile::exists (movie.poster)){
                pixmap.load (movie.poster);
            }else{
                pixmap.load ("/opt/MediaFinder/poster.png");
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
                            pixmap.load("/opt/MediaFinder/poster.png");
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

    if(QFile::exists (show.poster)){
        posterPixmap.load (show.poster);
    }else{
        posterPixmap.load ("/opt/MediaFinder/poster.png");
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
    ui->overviewTVShowText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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
    ui->infoCountSText->setText(QString::number(show.numberOfSeasons));
    ui->infoCountEText->setText(QString::number(show.numberOfEpisodes));
}

void MainWindow::fillTvShowEpisodeForm(int id, int seasonsNumber, int episodeNumber)
{
    ShowInfo showTV = dbmanager->getShowTVShowByID(id);
    if(this->NameShowLoaded!=showTV.nameShow){
        this->fillTvShowForm(id);
    }
    EpisodeInfo episode = showTV.getEpisode(seasonsNumber, episodeNumber);

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

    QPixmap posterPixmap;

    if(QFile::exists (episode.still_path)){
        posterPixmap.load (episode.still_path);
    }else{
        posterPixmap.load ("/opt/MediaFinder/poster.png");
    }

    QPixmap scaledPixmap = posterPixmap.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->posterEpisode->setPixmap(scaledPixmap);
    ui->posterEpisode->setWindowTitle(episode.episodeTitle);
}

void MainWindow::reloadSettings()
{
    delete this->settings; // Освобождаем память от старых настроек
    this->settings = dbmanager->getAllSettings(); // Получаем новые настройки из базы данных
}





// void MainWindow::switchLayoutTVShow(int index)
// {
//     switch (index) {
//     case 1: {
//         // Удаляем EpisodeLayout из текущего родителя, если он есть
//         if (EpisodeLayout->parentWidget()) {
//             EpisodeLayout->parentWidget()->layout()->removeItem(EpisodeLayout);
//         }
//         this->setLayoutVisibility(ui->ShowInfoLayout, false);
//         this->setLayoutVisibility(ui->EpisodeInfoLayout, true);
//         // Удаляем TvShowLayout из TVShowInfoLayout, если он там есть
//         ui->TVShowInfoLayout->removeItem(TvShowLayout);

//         // Добавляем TvShowLayout в TVShowInfoLayout
//         ui->TVShowInfoLayout->addLayout(TvShowLayout);
//         break;
//     }
//     case 2: {
//         // Удаляем TvShowLayout из текущего родителя, если он есть
//         if (TvShowLayout->parentWidget()) {
//             TvShowLayout->parentWidget()->layout()->removeItem(TvShowLayout);
//         }
//         this->setLayoutVisibility(ui->ShowInfoLayout, true);
//         this->setLayoutVisibility(ui->EpisodeInfoLayout, false);
//         // Удаляем EpisodeLayout из TVShowInfoLayout, если он там есть
//         ui->TVShowInfoLayout->removeItem(EpisodeLayout);

//         // Добавляем EpisodeLayout в TVShowInfoLayout
//         ui->TVShowInfoLayout->addLayout(EpisodeLayout);
//         break;
//     }
//     }
// }

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
}

void MainWindow::slotEndSearch()
{

    switch (ui->tabMainWindow->currentIndex ()) {
    case 0:{ //Movie
        this->updateCollections("Movie");
        // ui->listMovieLibrary->clearSelection();
        // QTreeWidgetItem *itemMovie = ui->listMovieLibrary->topLevelItem(0); // Первый элемент в дереве
        // if (itemMovie) {
        //     qDebug() << "Selecting Item 1";
        //     itemMovie->setSelected(false); // Снимаем выделение
        //     itemMovie->setSelected(true);  // Снова выделяем элемент
        //     ui->listMovieLibrary->scrollToItem(itemMovie); // Прокручиваем к выбранному элементу
        // }
        }
        break;
    case 1:{ //TV
            this->updateCollections("TV");
            // ui->listTVLibrary->clearSelection();

            // QTreeWidgetItem *itemTV = ui->listTVLibrary->topLevelItem(0); // Первый элемент в дереве
            // if (itemTV) {
            //     qDebug() << "Selecting Item 1";
            //     itemTV->setSelected(false); // Снимаем выделение
            //     itemTV->setSelected(true);  // Снова выделяем элемент
            //     ui->listTVLibrary->scrollToItem(itemTV); // Прокручиваем к выбранному элементу
            // }
        }
        break;
    }

}

void MainWindow::slotChangetSelection()
{
    qDebug() << "Выделение снято";
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
    // dialogSettingsApp->setAttribute(Qt::WA_DeleteOnClose);
    dialogSettingsApp->setWindowTitle("Settings - MediaFinder");
    // Показываем диалоговое окно при нажатии на кнопку
    dialogSettingsApp->show();
}

void MainWindow::on_loadMediaButton_clicked()
{
    //Открытие окна поиска
    this->searchMedia = new SearchMedia(this, this->mediaLibrary,this->dbmanager,progressBar, this->settings);
    connect(this->searchMedia, &SearchMedia::windowClosed, this, &MainWindow::onDialogClosed);
    connect(this->searchMedia, &SearchMedia::endSearch, this,  &MainWindow::slotEndSearch);
    MainWindow::setDisabled (true);
    this->searchMedia->setDisabled (false);
    // this->searchWindow->setAttribute(Qt::WA_DeleteOnClose);
    this->searchMedia->setWindowTitle("Search media - MediaFinder");
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
            qDebug() << idMediaDB;
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
        qDebug() << idMediaDB;
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

