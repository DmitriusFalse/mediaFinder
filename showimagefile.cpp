#include "showimagefile.h"
#include "ui_showimagefile.h"
#include <QApplication>
#include <QScreen>

ShowImageFile::ShowImageFile(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ShowImageFile)
{
    ui->setupUi(this);

    ui->imageView->setMaximumHeight(800);
    ui->imageView->setMaximumWidth(600);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        // Получаем размеры экрана
        QRect screenGeometry = screen->geometry();
        this->screenWidth = screenGeometry.width();
        this->screenHeight = screenGeometry.height();
        ui->imageView->setMaximumWidth(this->screenWidth*0.9);
        ui->imageView->setMaximumHeight(this->screenHeight*0.9);
    }

}

ShowImageFile::~ShowImageFile()
{
    delete ui;
}

void ShowImageFile::setImage(QString path, QString title)
{
    if(image!=nullptr){
        delete image;
    }
    image = new QPixmap;
    image->load(path);
    QSize widgetSize = ui->imageView->size();
    qDebug() << widgetSize;
    QPixmap scaledImage = image->scaled(QSize{this->screenWidth,this->screenHeight}, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->imageView->setPixmap(scaledImage);
    this->setWindowTitle(title);
}

void ShowImageFile::on_pushButton_clicked()
{
    this->close();
}

