#include "dialogshowprogress.h"
#include "ui_dialogshowprogress.h"

DialogShowProgress::DialogShowProgress(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogShowProgress)
{
    ui->setupUi(this);
    ui->progressBar->setValue (0);
}

DialogShowProgress::~DialogShowProgress()
{
    delete ui;
}

void DialogShowProgress::closeProgres()
{
    emit emitCloseProgres();
    this->close();
}

void DialogShowProgress::updateProgres()
{
    int val = ui->progressBar->value ();
    if(val+10>100){
        val = 0;
    }else{
        val +=10;
    }
    ui->progressBar->setValue (val);
}

void DialogShowProgress::setTextProgres(QString text)
{
    if(text != ""){
        text = ": "+text;
    }else{
        text = "";
    }
    ui->progressBar->setFormat(mainLineMessage+text);
    this->updateProgres();
}

void DialogShowProgress::setMainLineMessage(const QString &newMainLineMessage)
{
    mainLineMessage = newMainLineMessage;
    this->setTextProgres("");
}

void DialogShowProgress::slotUpdateProgress()
{
    int val = ui->progressBar->value ();
    if(val+10>100){
        val = 0;
    }else{
        val +=10;
    }
    ui->progressBar->setValue (val);
}
