#ifndef SHOWIMAGEFILE_H
#define SHOWIMAGEFILE_H

#include <QDialog>

namespace Ui {
class ShowImageFile;
}

class ShowImageFile : public QDialog
{
    Q_OBJECT

public:
    explicit ShowImageFile(QWidget *parent = nullptr);
    ~ShowImageFile();
    void setImage(QString path, QString title);

private slots:
    void on_pushButton_clicked();

private:
    Ui::ShowImageFile *ui;
    QPixmap *image = nullptr;
};

#endif // SHOWIMAGEFILE_H
