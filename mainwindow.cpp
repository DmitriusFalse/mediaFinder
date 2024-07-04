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
    this->searchMedia = new SearchMedia(this, this->mediaLibrary,this->dbmanager);

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

    connect(this->searchMedia, &SearchMedia::windowClosed, this, &MainWindow::onDialogClosed);
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

            QPixmap pixmap(movie.poster);
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
            mainItem->setText(1, show.seriesName);
            QPixmap pixmap(show.posterPath);
            QLabel *imageLabel = new QLabel();
            imageLabel->setPixmap(pixmap.scaled(90, 128));
            ui->listTVLibrary->setItemWidget(mainItem, 0, imageLabel);

            for (auto& episodes : show.Episodes) {
                QTreeWidgetItem *subItem1 = new QTreeWidgetItem(mainItem);
                subItem1->setText(1, episodes.episodeTitle+" S"+QString::number (episodes.seasonsNumber)+" E"+QString::number (episodes.episodeNumber));
            }

        // ui->listTVLibrary->expandAll();
        // ui->listTVLibrary->setWindowTitle("Пример QTreeWidget");
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
    MainWindow::setDisabled (true);
    this->searchMedia->setDisabled (false);
    // this->searchWindow->setAttribute(Qt::WA_DeleteOnClose);
    this->searchMedia->setWindowTitle("Search media - MediaFinder");

    switch (ui->tabMainWindow->currentIndex ()) {
    case 0:{ //Movie
        QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();
        this->searchMedia->setSearchWord (selectedItem->text(1));
        this->searchMedia->fillFields ("Movie");
        break;
    }
    case 1:{ //TV
        QTreeWidgetItem *selectedItem = ui->listTVLibrary->currentItem();
        this->searchMedia->setSearchWord (selectedItem->text(1));
        this->searchMedia->fillFields ("TV");
        break;
    }
    }


    this->searchMedia->show();
}

