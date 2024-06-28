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
    ui->setupUi(this);
    this->m_dbmanager = new DBManager(this);
    this->settingsData = new SettingsData(this->m_dbmanager);
    this->mediaLibrary = new MediaLibrary(this, this->m_dbmanager, settingsData);

    ui->PBRefreshLibrary->hide();
    ui->listMovieLibrary->setStyleSheet("QTreeWidget::item { height: 128px; }");
    ui->listMovieLibrary->setHeaderLabels({"Афиша","Название"});
    ui->listMovieLibrary->setColumnWidth(0,128);

    ui->listTVLibrary->setHeaderLabels({"Афиша","Название"});
    ui->listTVLibrary->setColumnWidth(0,128);

    QScrollBar *verticalScrollBar = ui->listMovieLibrary->verticalScrollBar ();
    verticalScrollBar->setStyleSheet("width: 30px;");

    connect(ui->listMovieLibrary, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onTreeWidgetItemSelected);
    connect(this->mediaLibrary,
            &MediaLibrary::updateProgressBarUI,
            this,
            &MainWindow::slotUptateProgressBar);
    connect(this->m_dbmanager,
            &DBManager::signalUpdateProgresBar,
            this,
            &MainWindow::slotUptateProgressBar);
    connect(this->m_dbmanager,
            &DBManager::signalUpdateMainWindow,
            this,
            &MainWindow::slotUpdateListLibrary);

    ui->listMovieLibrary->setAlternatingRowColors(true);
    this->updateCollections("Movie");
    this->updateCollections("TV");



}

MainWindow::~MainWindow()
{
    delete ui;
    // delete this->m_dbmanager;
    delete this->settingsData;
    delete this->mediaLibrary;
}

void MainWindow::onDialogClosed()
{
    // Включаем главное окно после закрытия диалогового окна
    setDisabled(false);
}


void MainWindow::on_refreshLibrary_clicked()
{
    ui->PBRefreshLibrary->show ();
    ui->PBRefreshLibrary->setFormat ("Обновление коллекции фильмов!");
    mediaLibrary->refsreshCollectionMovie();
}

void MainWindow::onTreeWidgetItemSelected()
{
    QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();

    QString hiddenData = selectedItem->data(0, Qt::UserRole).toString();
    QMessageBox::information(this, "Заголовок", hiddenData);

}

void MainWindow::updateCollections(QString type)
{

    QStringList action = {"Movie", "TV"};
    switch (action.indexOf(type)) {
    case 0:{
        movieCollections movies= mediaLibrary->getMovieInBase ("short");
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
        QTreeWidgetItem *mainItem = new QTreeWidgetItem(ui->listTVLibrary);

        mainItem->setText(1, "Основной элемент");

        QPixmap pixmap("/opt/MediaFinder/poster.png");
        QLabel *imageLabel = new QLabel();
        imageLabel->setPixmap(pixmap.scaled(90, 128));

        ui->listTVLibrary->setItemWidget(mainItem, 0, imageLabel);

        // Добавляем подпункты
        QTreeWidgetItem *subItem1 = new QTreeWidgetItem(mainItem);
        subItem1->setText(1, "Подпункт 1");



        // Разворачиваем все элементы для отображения
        ui->listTVLibrary->expandAll();

        ui->listTVLibrary->setWindowTitle("Пример QTreeWidget");

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
void MainWindow::on_openSettings_clicked()
{
    setDisabled(true);
    MainWindow::dialogSettingsApp = new SettingsApp(this,m_dbmanager, settingsData);

    connect (this->dialogSettingsApp,
            &SettingsApp::signalUpdateListCollection,
            this,
            &MainWindow::slotUpdateListLibraries);
    dialogSettingsApp->setAttribute(Qt::WA_DeleteOnClose);
    dialogSettingsApp->setWindowTitle("Settings - MediaFinder");
    // Показываем диалоговое окно при нажатии на кнопку
    dialogSettingsApp->show();

    connect(dialogSettingsApp, &SettingsApp::destroyed, this, &MainWindow::onDialogClosed);
}

