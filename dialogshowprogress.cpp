#include "dialogshowprogress.h"
#include "ui_dialogshowprogress.h"

DialogShowProgress::DialogShowProgress(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogShowProgress)
{
    ui->setupUi(this);
}

DialogShowProgress::~DialogShowProgress()
{
    delete ui;
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
