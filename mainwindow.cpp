#include "mainwindow.h"
#include "settingsapp.h"
#include "ui_mainwindow.h"
#include "settingsdata.h"
#include <QDir>
#include <QIcon>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<libraryItem>("libraryItem");
    QFileInfo file(".");
    qDebug() << "Path: " << file.canonicalFilePath();

    connect(ui->listMovieLibrary, &QTreeWidget::itemSelectionChanged, this, &MainWindow::onTreeWidgetItemSelected);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

    setDisabled(true);
    MainWindow::dialog = new SettingsApp(this);
    qDebug() << "Clicked";
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("Settings - MediaFinder");
    // Показываем диалоговое окно при нажатии на кнопку
    dialog->show();

    connect(dialog, &SettingsApp::destroyed, this, &MainWindow::onDialogClosed);
}
void MainWindow::onDialogClosed()
{
    // Включаем главное окно после закрытия диалогового окна
    setDisabled(false);
}


void MainWindow::on_refreshLibrary_clicked()
{
    movieCollections movies = mediaLibrary->getRefsreshCollectionMovie();
    ui->listMovieLibrary->clear();
    ui->listMovieLibrary->setStyleSheet("QTreeWidget::item { height: 128px; }");
    ui->listMovieLibrary->setHeaderLabels({"Афиша","Название"});
    ui->listMovieLibrary->setColumnWidth(0,128);
    for (auto& movie : movies.items) {

        QTreeWidgetItem *item1 = new QTreeWidgetItem(ui->listMovieLibrary);

        QPixmap pixmap(movie.pathIcon);
        QLabel *imageLabel = new QLabel();
        imageLabel->setPixmap(pixmap.scaled(90, 128));

        ui->listMovieLibrary->setItemWidget(item1, 0, imageLabel);

        item1->setText(1, movie.name);

    }

}

void MainWindow::onTreeWidgetItemSelected()
{
    QTreeWidgetItem *selectedItem = ui->listMovieLibrary->currentItem();
    QMessageBox::information(this, "Заголовок", selectedItem->text(1));

}


