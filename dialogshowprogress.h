#ifndef DIALOGSHOWPROGRESS_H
#define DIALOGSHOWPROGRESS_H

#include <QDialog>

namespace Ui {
class DialogShowProgress;
}

class DialogShowProgress : public QDialog
{
    Q_OBJECT

public:
    explicit DialogShowProgress(QWidget *parent = nullptr);
    ~DialogShowProgress();

    void closeProgres();

    void updateProgres();
    void reset();
    void setTextProgres(QString text);

    void setMainLineMessage(const QString &newMainLineMessage);

private:
    Ui::DialogShowProgress *ui;
    QString mainLineMessage;
public slots:
    void slotUpdateProgress();
signals:
    void emitCloseProgres();
};

#endif // DIALOGSHOWPROGRESS_H
