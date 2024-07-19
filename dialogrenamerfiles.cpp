#include "dialogrenamerfiles.h"
#include "dbmanager.h"
#include "ui_dialogrenamerfiles.h"
#include <QFileInfo>
#include <QRegularExpression>


DialogRenamerFiles::DialogRenamerFiles(QWidget *parent, DBManager *db)
    : QDialog(parent)
    , ui(new Ui::DialogRenamerFiles)
    , dbmanager(db)
{
    ui->setupUi(this);
// oldListMovie

}

DialogRenamerFiles::~DialogRenamerFiles()
{
    delete ui;
}

void DialogRenamerFiles::setTypeMedia(int index)
{
    ui->tabRenamer->setCurrentIndex(index);
    switch (index) {
    case 0:{
        ui->tabRenamer->setTabEnabled(1, false);
        ui->tabRenamer->setTabEnabled(0, true);
    }break;
    case 1:{
        ui->tabRenamer->setTabEnabled(0, false);
        ui->tabRenamer->setTabEnabled(1, true);
    }break;
    }
}

void DialogRenamerFiles::setMediaData(MovieInfo mov)
{
    this->movie = mov;
    this->replacePlaceholders = new Placeholders(this->movie);
    QFileInfo fileinfo(this->movie.path);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->oldListMovie);
    QString nameMovie = fileinfo.completeBaseName();
    newItem->setText(0, nameMovie);
    ui->patternMovieEdit->setText(nameMovie);

    this->oldIitem = new QTreeWidgetItem(ui->newListMovie);

    QStringList description;
    description.append("<table>");
    while (this->replacePlaceholders->next()) {
        QString key = this->replacePlaceholders->currentKey();
        QString desc = this->replacePlaceholders->getFieldDescription(key);
        QString value = this->replacePlaceholders->getValue(key);

        // Формируем строку для каждого элемента таблицы
        QString row = "<tr>"
                          "<td><b>Описание:</b> " + desc + "</td>"
                          "<td><b>Подстановочное слово:</b> " + key + "</td>"
                          "<td><b>Данные:</b> " + value + "</td>"
                      "</tr>";
        // Добавляем сформированную строку в общий текст таблицы
        description.append(row);
    }
    description += "</table>";
    ui->patternMovieDescription->setHtml(description.join(""));

    QSize sizeText = ui->patternMovieDescription->size();
    int countRow = description.size()-2;
    if(countRow>=2){
        sizeText.setHeight(countRow*25);
    }else if(countRow==1){
        sizeText.setHeight(25);
    }
    ui->patternMovieDescription->setMinimumHeight(sizeText.height());
    ui->patternMovieDescription->setMaximumHeight(sizeText.height());

}

void DialogRenamerFiles::setMediaData(ShowInfo sh)
{

}

void DialogRenamerFiles::on_patternMovieEdit_textChanged(const QString &arg1)
{
    qDebug() << ui->patternMovieEdit->text();
    this->changeName(ui->patternMovieEdit->text());
}

void DialogRenamerFiles::changeName(QString pattern)
{
    newFileName = this->replacePattern(pattern);
    ui->newListMovie->clear();
    // delete this->oldIitem;
    // Пересоздаем элемент oldIitem после очистки QTreeWidget
    this->oldIitem = new QTreeWidgetItem(ui->newListMovie);
    this->oldIitem->setText(0, newFileName);
}

QString DialogRenamerFiles::replacePattern(const QString &input)
{
    QString result = input;
    QRegularExpression placeholderPattern(":(\\w+)");

    // Используем QRegularExpressionMatchIterator для итерации по всем совпадениям
    QRegularExpressionMatchIterator matchIterator = placeholderPattern.globalMatch(result);

    int offset = 0; // Смещение для корректной замены в исходной строке
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString placeholder = match.captured(0); // Захватываем имя placeholder

        QString value = replacePlaceholders->getValue(placeholder);
        qDebug() << "placeholder " << placeholder;
        if (value.isEmpty()) {
            // Если значение пустое, оставляем метку на месте
            continue;
        }

        // Находим позицию, где начинается совпадение в исходной строке
        int matchStart = match.capturedStart(0);
        // Находим длину совпадения
        int matchLength = match.capturedLength(0);

        // Заменяем метку на значение
        result.replace(matchStart + offset, matchLength, value);

        // Обновляем смещение для корректной работы с исходной строкой
        offset += value.length() - matchLength;
    }

    return result;
}

QString DialogRenamerFiles::renameFile(const QString &filePath, const QString &newName)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix();

    QFileInfo newFileInfo(newFileName);
    QString newFileNameWithExtension = newFileName;

    // Проверяем, есть ли у нового имени файла расширение
    if (newFileInfo.suffix().isEmpty()) {
        newFileNameWithExtension += "." + extension;
    }

    QString newFilePath = fileInfo.absolutePath() + "/" + newFileNameWithExtension;

    QFile file(filePath);
    if (file.rename(newFilePath)) {
        return newFilePath; // Возвращаем полный путь к новому файлу при успехе
    } else {
        qDebug() << "Не удалось переименовать файл:" << file.errorString();
        return filePath; // Возвращаем полный путь к старому файлу при провале
    }
}


void DialogRenamerFiles::on_renameMovieButton_clicked()
{
    QString oldPath = movie.path;
    QString oldPoster = movie.poster;
    qDebug() << newFileName;
    QString newFilePath = this->renameFile(oldPath, newFileName);
    QString newFilePoster = this->renameFile(oldPoster, newFileName);
    dbmanager->updateMovieColumn("path", newFilePath, movie.id);
    dbmanager->updateMovieColumn("poster", newFilePoster, movie.id);
    this->close();
    emit signalFinishRename("Movie", movie.id);
}

