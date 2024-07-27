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
    int screenWidth = 800;
    int screenHeight = 600;
};

#endif // SHOWIMAGEFILE_H
