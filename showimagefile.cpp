#include "showimagefile.h"
#include "ui_showimagefile.h"

ShowImageFile::ShowImageFile(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ShowImageFile)
{
    ui->setupUi(this);
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
    ui->imageView->setPixmap(*image);
    this->setWindowTitle(title);
}

void ShowImageFile::on_pushButton_clicked()
{
    this->close();
}

