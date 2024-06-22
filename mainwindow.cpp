#include "mainwindow.h"
#include "settingsapp.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

}

