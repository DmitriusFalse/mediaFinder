#include "dialogrenamerfiles.h"
#include "dbmanager.h"
#include "ui_dialogrenamerfiles.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QDir>


DialogRenamerFiles::DialogRenamerFiles(QWidget *parent, DBManager *db)
    : QDialog(parent)
    , ui(new Ui::DialogRenamerFiles)
    , dbmanager(db)
{
    ui->setupUi(this);
// oldListMovie
    this->checkNewFoldersEpisodes = true;

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
    newItem->setText(0, fileinfo.baseName());

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
            QFileInfo episodeFile(episodeInfo.filePath);
            subItem1->setText(0, episodeFile.baseName());
            subItem1->setData(1,Qt::UserRole,episodeInfo.seasonsNumber);
            subItem1->setData(2,Qt::UserRole,episodeInfo.episodeNumber);
            subItem1->setData(3,Qt::UserRole,episodeInfo.ID);
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
    if(arg1==""){
        ui->renameMovieButton->setDisabled(true);
    }else{
        ui->renameMovieButton->setDisabled(false);
    }
    this->changeNameMovie(arg1);
}

void DialogRenamerFiles::on_patternTVEdit_textChanged(const QString &arg1)
{
    if(arg1==""){
        ui->renameTVButton->setDisabled(true);
    }else{
        ui->renameTVButton->setDisabled(false);
    }
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
            subItem1->setData(3,Qt::UserRole,episodeInfo.ID);
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
    if(!QFile::exists(filePath)){
        return QString();
    }
    QFileInfo oldFilePath(filePath);

    QString newFilePath = oldFilePath.canonicalPath()+"/"+newName+"."+oldFilePath.suffix();
    QFileInfo newFileInfo(newFilePath);
    if(!QFile::exists(newFileInfo.absolutePath())){
        QDir dir;
        dir.mkpath(newFileInfo.absolutePath());
    }


    if(filePath!=newFilePath){
        QFile file(filePath);
        if (file.rename(newFilePath)) {
            return newFilePath; // Возвращаем полный путь к новому файлу при успехе
        } else {
            qDebug() << "Не удалось переименовать файл:" << file.errorString();
            qDebug() << "Old File: " << filePath;
            qDebug() << "New File: " << newFilePath;
            return filePath; // Возвращаем полный путь к старому файлу при провале
        }
    }else{
        return filePath;
    }
    return QString();
}


void DialogRenamerFiles::on_renameMovieButton_clicked()
{
    QString oldPath = movie.path;
    QString oldPoster = movie.poster;

    QString newFilePath = this->renameFile(oldPath, newFileName);
    QString newFilePoster = this->renameFile(oldPoster, newFileName);
    dbmanager->updateMovieColumn("path", newFilePath, movie.id);
    dbmanager->updateMovieColumn("poster", newFilePoster, movie.id);
    this->close();
    emit signalFinishRename("Movie", movie.id);
}


void DialogRenamerFiles::on_renameTVButton_clicked()
{

    QTreeWidgetItem *topLevelItem = ui->newListTV->topLevelItem(0);

    for (int i = 0; i < topLevelItem->childCount(); ++i) {
        QTreeWidgetItem *childItem = topLevelItem->child(i);
        QString newName = childItem->text(0);
        uint season = childItem->data(1,Qt::UserRole).toUInt();
        uint episode = childItem->data(2,Qt::UserRole).toUInt();

        if(this->checkNewFoldersEpisodes){
            newName = "Season "+QString::number(season)+"/"+newName;
        }

        EpisodeInfo infoEpisode = this->showTv.getEpisode(season, episode);
        QString oldPath = infoEpisode.filePath;
        int  id = childItem->data(3,Qt::UserRole).toInt();


        QString newFileName = this->renameFile(oldPath, newName);
        dbmanager->updateEpisodeColumn("file", newFileName, id);

        QString oldPoster = infoEpisode.still_path;
        if(oldPoster!=""){
            QString newFilePoster = this->renameFile(oldPoster, newName);
            dbmanager->updateEpisodeColumn("Poster", newFilePoster, id);
        }

    }
    this->close();
    emit signalFinishRename("TV", this->showTv.ID);
}


void DialogRenamerFiles::on_folderSeasonsCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    this->checkNewFoldersEpisodes = (arg1 == Qt::Checked);
    qDebug() << this->checkNewFoldersEpisodes;
}

