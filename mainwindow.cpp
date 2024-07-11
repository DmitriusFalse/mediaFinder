#include "mainwindow.h"
#include "settingsapp.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QDir>
#include <QIcon>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{

    qRegisterMetaType<libraryItem>("libraryItem");
    qRegisterMetaType<movieCollections>("movieCollections");
    qRegisterMetaType<movieItem>("movieItem");
    qRegisterMetaType<TVCollection>("TVCollection");
    qRegisterMetaType<ShowInfo>("SerialInfo");
    // qRegisterMetaType<SeasonInfo>("SeasonInfo");
    qRegisterMetaType<EpisodeInfo>("EpisodeInfo");
    ui->setupUi(this);
    this->dbmanager = new DBManager(this);
    this->settingsData = new SettingsData(this->dbmanager);
    this->mediaLibrary = new MediaLibrary(this, this->dbmanager, settingsData);
    this->progressBar = new DialogShowProgress;


    this->dialogSettingsApp = new SettingsApp(this,dbmanager, settingsData);



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
    QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();

    QString hiddenData = selectedItem->data(0, Qt::UserRole).toString();
    QMessageBox::information(this, "Заголовок", hiddenData);

}

void MainWindow::clickTreeWidgetTV()
{
    qDebug() << "Click TV";
}

void MainWindow::updateCollections(QString type)
{

    QStringList action = {"Movie", "TV"};
    switch (action.indexOf(type)) {
        case 0:{
            movieCollections movies= mediaLibrary->getMovieCollection ("short");
            ui->listMovieLibrary->clear();

            for (auto& movie : movies.items) {

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
            TVCollection tvcols = mediaLibrary->getTVCollection ("short");
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
                mainItem->setData(0, Qt::UserRole, show.ID);

                foreach (const uint seasonNumber, show.Episodes.keys()) {
                    const QMap<uint, EpisodeInfo>& episodes = show.Episodes[seasonNumber];
                    foreach (const uint episodeNumber, episodes.keys()) {
                        const EpisodeInfo& episodeInfo = episodes[episodeNumber];
                        QTreeWidgetItem *subItem1 = new QTreeWidgetItem(mainItem);
                        subItem1->setText(1, episodeInfo.episodeTitle+" S"+QString::number (episodeInfo.seasonsNumber)+" E"+QString::number (episodeInfo.episodeNumber));
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


            break;
            }
        case 1:{ //tv
                for (int index = 0; index < ui->listTVLibrary->topLevelItemCount(); ++index) {
                    QTreeWidgetItem *item = ui->listTVLibrary->topLevelItem(index);
                    int ID = item->data(0, Qt::UserRole).toInt();
                    if(id==ID){

                        QList<QTreeWidgetItem*> children = item->takeChildren();
                        for (QTreeWidgetItem *child : children) {
                            delete child;
                        }

                        ShowInfo show = mediaLibrary->getShowInfoByID ("short", id);
                        QPixmap pixmap;
                        if(QFile::exists (show.poster)){
                            pixmap.load (show.poster);
                        }else{
                            pixmap.load ("/opt/MediaFinder/poster.png");
                        }
                        QLabel *imageLabel = new QLabel();
                        imageLabel->setPixmap(pixmap.scaled(90, 128));
                        ui->listTVLibrary->setItemWidget(item, 0, imageLabel);

                        foreach (const uint seasonNumber, show.Episodes.keys()) {
                            const QMap<uint, EpisodeInfo>& episodes = show.Episodes[seasonNumber];
                            foreach (const uint episodeNumber, episodes.keys()) {
                                const EpisodeInfo& episodeInfo = episodes[episodeNumber];
                                QTreeWidgetItem *subItem1 = new QTreeWidgetItem(item);
                                subItem1->setText(1, episodeInfo.episodeTitle+" S"+QString::number (episodeInfo.seasonsNumber)+" E"+QString::number (episodeInfo.episodeNumber));
                            }
                        }
                        break;
                    }
                }



            break;
            }
    }
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
}

void MainWindow::slotChangetSelection()
{
    auto treeWidget = qobject_cast<QTreeWidget*>(sender());
    if (treeWidget->selectedItems().isEmpty()) {
        // Код, который нужно выполнить при снятии выделения
        qDebug() << "Выделение снято";
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
    this->searchMedia = new SearchMedia(this, this->mediaLibrary,this->dbmanager,progressBar);
    connect(this->searchMedia, &SearchMedia::windowClosed, this, &MainWindow::onDialogClosed);
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
        selectedItem = ui->listTVLibrary->currentItem();
        int idMediaDB = selectedItem->data(0, Qt::UserRole).toInt();
        qDebug() << idMediaDB;
        this->searchMedia->setIdTVDB (idMediaDB);
        this->searchMedia->setSearchWord (selectedItem->text(1));
        this->searchMedia->fillFields ("TV");
        break;
    }
    }


    this->searchMedia->show();
}

