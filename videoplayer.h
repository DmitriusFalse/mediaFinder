#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QDialog>
#include "movieCollections.h"

namespace Ui {
class videoPlayer;
}

class videoPlayer : public QDialog
{
    Q_OBJECT

public:
    explicit videoPlayer(QWidget *parent = nullptr);
    ~videoPlayer();
    void open();
    void setVideoPlayList(QList<Videos> videoList);

private slots:
    void on_closeVideoPlayer_clicked();

private:
    Ui::videoPlayer *ui;
    QList<Videos> videos;
    void clickItemTreeVideo();
    void clearVideo();
signals:
    void closePlayer();
protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // VIDEOPLAYER_H
