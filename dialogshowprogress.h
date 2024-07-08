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

private:
    Ui::DialogShowProgress *ui;
public slots:
    void slotUpdateProgress();
};

#endif // DIALOGSHOWPROGRESS_H
