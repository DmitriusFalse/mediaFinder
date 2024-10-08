#include "videoplayer.h"
#include "ui_videoplayer.h"
#include <QWebEngineView>
#include <QCloseEvent>
videoPlayer::videoPlayer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::videoPlayer)
{
    ui->setupUi(this);
    connect(ui->playlistVideo, &QTreeWidget::itemSelectionChanged, this, &videoPlayer::clickItemTreeVideo);
}

videoPlayer::~videoPlayer()
{
    delete ui;
}

void videoPlayer::open()
{
    this->clearVideo();
    this->show();
}

void videoPlayer::setVideoPlayList(QList<Videos> videoList)
{
    this->videos = videoList;
    ui->playlistVideo->clear();
    ui->scrollAreaVideo->setWidget(nullptr);
    for (const Videos &video : videoList) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->playlistVideo);
        if(video.name.size()>0){
            item->setText(0, video.name);
        }else{
            item->setText(0, tr("Видео"));
        }
        item->setData(0, Qt::UserRole, video.key);
    }

}

QList<Videos> videoPlayer::getVideoPlayList()
{
    return this->videos;
}

void videoPlayer::on_closeVideoPlayer_clicked()
{
    emit closePlayer();
    // this->close();
}

void videoPlayer::clickItemTreeVideo()
{

    QTreeWidgetItem *selectedItem = ui->playlistVideo->currentItem();
    QString keyVideo = selectedItem->data(0, Qt::UserRole).toString();
    QWebEngineView *view = new QWebEngineView(this);

    if(keyVideo.size()>0){
        view->setUrl(QUrl("https://www.youtube.com/embed/"+keyVideo));
        ui->scrollAreaVideo->setWidget(view);
    }

}

void videoPlayer::clearVideo()
{
    QList<QWidget*> children = ui->scrollAreaVideo->widget()->findChildren<QWidget*>();

    // Удаляем все дочерние виджеты
    for (QWidget *child : children) {
        delete child;
    }
}

void videoPlayer::closeEvent(QCloseEvent *event)
{
    emit closePlayer();
    event->accept();
}

void videoPlayer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        this->close();  // Закрыть диалоговое окно при нажатии клавиши Esc
    } else {
        QDialog::keyPressEvent(event);  // Передать событие базовому классу
    }
}

