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
    this->replacePlaceholdersMovie = new PlaceholdersMovie(this->movie);
    QFileInfo fileinfo(this->movie.path);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->oldListMovie);
    QString nameMovie = fileinfo.completeBaseName();
    newItem->setText(0, nameMovie);

    this->oldIitem = new QTreeWidgetItem(ui->newListMovie);

    QStringList description;
    description.append("<table>");
    while (this->replacePlaceholdersMovie->next()) {
        QString key = this->replacePlaceholdersMovie->currentKey();
        QString desc = this->replacePlaceholdersMovie->getFieldDescription(key);
        QString value = this->replacePlaceholdersMovie->getValue(key);

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

    ui->patternMovieEdit->setText(":name-:release_date");
}

void DialogRenamerFiles::setMediaData(ShowInfo sh)
{
    this->showTv = sh;
    this->replacePlaceholdersTV = new PlaceholdersTV(this->showTv);
    QTreeWidgetItem *mainItem = new QTreeWidgetItem(ui->oldListTV);
    mainItem->setText(0, showTv.nameShow);
    mainItem->setData(0, Qt::UserRole, showTv.ID);
    uint countRow = 0;
    foreach (const uint seasonNumber, showTv.Episodes.keys()) {
        const QMap<uint, EpisodeInfo>& episodes = showTv.Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            const EpisodeInfo& episodeInfo = episodes[episodeNumber];
            QTreeWidgetItem *subItem1 = new QTreeWidgetItem(mainItem);
            subItem1->setText(0, episodeInfo.episodeTitle);
            subItem1->setData(1,Qt::UserRole,episodeInfo.seasonsNumber);
            subItem1->setData(2,Qt::UserRole,episodeInfo.episodeNumber);
            countRow++;
        }
    }
    mainItem->setExpanded(true);
    QStringList description;
    description.append("<table>");

    // Добавляем заголовок таблицы
    description.append("<tr><th>Описание</th><th>Подстановочное слово</th><th>Пример</th></tr>");

    // Обходим все заполненные ключи
    QSet<QString> processedKeys;
    for (const auto& field : this->replacePlaceholdersTV->fieldDescriptions) {
        qDebug ()<< "field:" << field;
        QString keyText = this->replacePlaceholdersTV->fieldDescriptions.key(field);
        QString desc = this->replacePlaceholdersTV->fieldDescriptions[keyText];
        QString value = this->replacePlaceholdersTV->getValue(1,1,keyText);
        QString row = "<tr>"
                            "<td><b>Описание:</b> " + desc + "</td>"
                            "<td><b>Подстановочное слово:</b> " + keyText + "</td>"
                            "<td><b>Пример:</b> " + value + "</td>"
                        "</tr>";
        // Добавляем сформированную строку в общий текст таблицы
        description.append(row);
    }
    description.append("</table>");
    ui->patternTVDescription->setHtml(description.join(""));

    ui->patternTVEdit->setText("S:seasonsNumberE:episodeNumber-:episodeTitle");
}

void DialogRenamerFiles::on_patternMovieEdit_textChanged(const QString &arg1)
{
    qDebug() << arg1;
    this->changeNameMovie(arg1);
}

void DialogRenamerFiles::on_patternTVEdit_textChanged(const QString &arg1)
{
    qDebug() << arg1;
    this->changeNameTv(arg1);
}

void DialogRenamerFiles::changeNameTv(QString pattern)
{
    ui->newListTV->clear();
    QTreeWidgetItem *mainItem = new QTreeWidgetItem(ui->newListTV);
    mainItem->setText(0, showTv.nameShow);
    mainItem->setData(0, Qt::UserRole, showTv.ID);
    uint countRow = 0;
    foreach (const uint seasonNumber, showTv.Episodes.keys()) {
        const QMap<uint, EpisodeInfo>& episodes = showTv.Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            const EpisodeInfo& episodeInfo = episodes[episodeNumber];
            QTreeWidgetItem *subItem1 = new QTreeWidgetItem(mainItem);

            subItem1->setText(0, this->replacePatternTV(pattern, seasonNumber, episodeNumber));

            subItem1->setData(1,Qt::UserRole,episodeInfo.seasonsNumber);
            subItem1->setData(2,Qt::UserRole,episodeInfo.episodeNumber);
            countRow++;
        }
    }
    mainItem->setExpanded(true);
}


void DialogRenamerFiles::changeNameMovie(QString pattern)
{
    newFileName = this->replacePatternMovie(pattern);
    ui->newListMovie->clear();
    // delete this->oldIitem;
    // Пересоздаем элемент oldIitem после очистки QTreeWidget
    this->oldIitem = new QTreeWidgetItem(ui->newListMovie);
    this->oldIitem->setText(0, newFileName);
}

QString DialogRenamerFiles::replacePatternMovie(const QString &input)
{
    QString result = input;

    for (const auto& field : this->replacePlaceholdersMovie->fieldDescriptions) {
        QString keyText = this->replacePlaceholdersMovie->fieldDescriptions.key(field);
        QString value = this->replacePlaceholdersMovie->getValue(keyText);
        result = result.replace(keyText, value);
    }
    return result;
}

QString DialogRenamerFiles::replacePatternTV(const QString &input, int Season, int Episode)
{
    QString result = input;
    for (const auto& field : this->replacePlaceholdersTV->fieldDescriptions) {
        QString keyText = this->replacePlaceholdersTV->fieldDescriptions.key(field);
        QString value = this->replacePlaceholdersTV->getValue(Season,Episode,keyText);
        result = result.replace(keyText, value);
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

