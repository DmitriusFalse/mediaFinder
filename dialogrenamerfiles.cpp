#include "dialogrenamerfiles.h"
#include "dbmanager.h"
#include "qxmlstream.h"
#include "ui_dialogrenamerfiles.h"
#include <QFileInfo>
#include <QRegularExpression>
#include <QDir>
#include <QKeyEvent>

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
    this->setWindowTitle(this->movie.name+" - Media Finder");
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
    this->setWindowTitle(this->showTv.nameShow+" - Media Finder");
    this->replacePlaceholdersTV = new PlaceholdersTV(this->showTv);
    QTreeWidgetItem *mainItem = new QTreeWidgetItem(ui->oldListTV);
    mainItem->setText(0, showTv.nameShow);
    mainItem->setData(0, Qt::UserRole, showTv.ID);

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
        }
    }
    mainItem->setExpanded(true);
    QStringList description;
    description.append("<table>");

    // Добавляем заголовок таблицы
    description.append("<tr><th>" + tr("Описание") + "</th><th>" + tr("Подстановочное слово") + "</th><th>" + tr("Пример") + "</th></tr>");

    // Обходим все заполненные ключи

    for (const auto& field : this->replacePlaceholdersTV->fieldDescriptions) {
        QString keyText = this->replacePlaceholdersTV->fieldDescriptions.key(field);
        QString desc = this->replacePlaceholdersTV->fieldDescriptions[keyText];
        QString value = this->replacePlaceholdersTV->getValue(1,1,keyText);
        QString row = "<tr>"
                            "<td><b>"+tr("Описание:")+"</b> " + desc + "</td>"
                            "<td><b>"+tr("Подстановочное слово:")+"</b> " + keyText + "</td>"
                            "<td><b>"+tr("Пример:")+"</b> " + value + "</td>"
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
    if(arg1==""){
        ui->renameButton->setDisabled(true);
    }else{
        ui->renameButton->setDisabled(false);
    }
    this->changeNameMovie(arg1);
}

void DialogRenamerFiles::on_patternTVEdit_textChanged(const QString &arg1)
{
    if(arg1==""){
        ui->renameButton->setDisabled(true);
    }else{
        ui->renameButton->setDisabled(false);
    }
    this->changeNameTv(arg1);
}

void DialogRenamerFiles::changeNameTv(QString pattern)
{
    ui->newListTV->clear();
    QTreeWidgetItem *mainItem = new QTreeWidgetItem(ui->newListTV);
    mainItem->setText(0, showTv.nameShow);
    mainItem->setData(0, Qt::UserRole, showTv.ID);

    foreach (const uint seasonNumber, showTv.Episodes.keys()) {
        const QMap<uint, EpisodeInfo>& episodes = showTv.Episodes[seasonNumber];
        foreach (const uint episodeNumber, episodes.keys()) {
            const EpisodeInfo& episodeInfo = episodes[episodeNumber];
            QTreeWidgetItem *subItem1 = new QTreeWidgetItem(mainItem);

            subItem1->setText(0, this->replacePatternTV(pattern, seasonNumber, episodeNumber));

            subItem1->setData(1,Qt::UserRole,episodeInfo.seasonsNumber);
            subItem1->setData(2,Qt::UserRole,episodeInfo.episodeNumber);
            subItem1->setData(3,Qt::UserRole,episodeInfo.ID);
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

    for (const QString& field : this->replacePlaceholdersMovie->fieldDescriptions) {
        QString keyText = this->replacePlaceholdersMovie->fieldDescriptions.key(field);
        QString value = this->replacePlaceholdersMovie->getValue(keyText);
        result = result.replace(keyText, value);
    }
    return result;
}

QString DialogRenamerFiles::replacePatternTV(const QString &input, int Season, int Episode)
{
    QString result = input;
    for (const QString& field : this->replacePlaceholdersTV->fieldDescriptions) {
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
            qDebug() << tr("Не удалось переименовать файл:") << file.errorString();
            return filePath; // Возвращаем полный путь к старому файлу при провале
        }
    }else{
        return filePath;
    }
    return QString();
}

QString DialogRenamerFiles::renameAndMoveFile(const QString &oldPath, const QString &newPath)
{
    if(!QFile::exists(oldPath)){
        return QString();
    }
    QFileInfo inOld(oldPath);
    QFileInfo toNew(newPath);
    if(!QFile::exists(toNew.absolutePath())){
        QDir dir;
        dir.mkpath(toNew.absolutePath());
    }
    if(inOld.absoluteFilePath()==toNew.absoluteFilePath()){
        return toNew.absoluteFilePath();
    }
    QFile file(oldPath);
    if (file.rename(newPath)) {
        return newPath; // Возвращаем полный путь к новому файлу при успехе
    } else {
        qDebug() << tr("Не удалось переименовать и переместить файл:") << file.errorString();
        return oldPath; // Возвращаем полный путь к старому файлу при провале
    }
    return oldPath;
}

bool DialogRenamerFiles::moveAndRemoveFile(const QString &sourceDirPath, const QString &destinationDirPath)
{
    if (sourceDirPath == destinationDirPath) {
        return true;
    }

    QDir sourceDir(sourceDirPath);
    if (!sourceDir.exists()) {
        qWarning() << "Source directory does not exist:" << sourceDirPath;
        return false;
    }

    QDir destinationDir(destinationDirPath);
    if (!destinationDir.exists()) {
        if (!destinationDir.mkpath(destinationDirPath)) {
            qWarning() << "Failed to create destination directory:" << destinationDirPath;
            return false;
        }
    }

    // Перемещение всех файлов
    foreach (const QString &fileName, sourceDir.entryList(QDir::Files)) {
        QString srcFilePath = sourceDir.filePath(fileName);
        QString dstFilePath = destinationDir.filePath(fileName);
        if (fileName.endsWith(".nfo", Qt::CaseInsensitive)) {
            continue;
        }
        if (!QFile::exists(dstFilePath)) {
            if (!QFile::rename(srcFilePath, dstFilePath)) {
                continue;
            }
        }
    }

    // Перемещение всех подкаталогов
    foreach (const QString &subDirName, sourceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString srcSubDirPath = sourceDir.filePath(subDirName);
        QString dstSubDirPath = destinationDir.filePath(subDirName);
        if (!this->moveAndRemoveFile(srcSubDirPath, dstSubDirPath)) {
            return false;
        }
    }

    // Удаление исходного каталога после успешного перемещения
    if (!sourceDir.removeRecursively()) {
        qWarning() << "Failed to remove source directory:" << sourceDirPath;
        return false;
    }

    qDebug() << "Contents moved successfully from" << sourceDirPath << "to" << destinationDirPath;
    return true;
}

void DialogRenamerFiles::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();  // Закрыть диалоговое окно при нажатии клавиши Esc
    } else {
        QDialog::keyPressEvent(event);  // Передать событие базовому классу
    }
}

void DialogRenamerFiles::renameMovie()
{
    QString oldPath = movie.path;
    QString oldPoster = movie.poster;

    QString newFilePath = this->renameFile(oldPath, newFileName);
    // QString newFilePoster = this->renameFile(oldPoster, newFileName);
    dbmanager->updateMovieColumn("path", newFilePath, movie.id);
    // dbmanager->updateMovieColumn("poster", newFilePoster, movie.id);
    if(this->checkMovieNFO){
        QFileInfo filepath(newFilePath);
        QFile file(filepath.path()+"/"+newFileName+".nfo");
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << tr("Не удалось открыть файл для записи:") << file.errorString();
            return;
        }
        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();

        // Начало корневого элемента
        xmlWriter.writeStartElement("movie");

        // Запись данных
        xmlWriter.writeStartElement("title");
        xmlWriter.writeCharacters(movie.name);
        xmlWriter.writeEndElement(); // Закрытие title

        xmlWriter.writeStartElement("original_title");
        xmlWriter.writeCharacters(movie.originalName);
        xmlWriter.writeEndElement(); // Закрытие original_title

        xmlWriter.writeStartElement("year");
        xmlWriter.writeCharacters(movie.release_date);
        xmlWriter.writeEndElement(); // Закрытие year

        while(movie.nextCrew()){
            Crew existCrew = movie.getCrew();
            if(!existCrew.hasRole("Director")){
                continue;
            }
            xmlWriter.writeStartElement("director");
            xmlWriter.writeCharacters(existCrew.name);
            xmlWriter.writeEndElement(); // Закрытие director
        }
        movie.resetIterator();
        QStringList listGenre = movie.genre.split(",");
        for (const QString &genre : listGenre) {
            xmlWriter.writeStartElement("genre");
            xmlWriter.writeCharacters(genre);
            xmlWriter.writeEndElement(); // Закрытие director
        }
        while(movie.nextCrew()){
            Crew existCrew = movie.getCrew();
            if(!existCrew.hasRole("Writer")){
                continue;
            }
            xmlWriter.writeStartElement("writer");
            xmlWriter.writeCharacters(existCrew.name);
            xmlWriter.writeEndElement(); // Закрытие writer
        }
        movie.resetIterator();

        xmlWriter.writeStartElement("plot");
        xmlWriter.writeCharacters(movie.overview);
        xmlWriter.writeEndElement(); // Закрытие writer

        xmlWriter.writeStartElement("crew");

        while(movie.nextCrew()){
            Crew existCrew = movie.getCrew();
            for (const QString& roleName : existCrew.role) {
                xmlWriter.writeStartElement("member");
                xmlWriter.writeTextElement("name", existCrew.name);
                xmlWriter.writeTextElement("role", roleName);
                // xmlWriter.writeTextElement("department", "Writing");
                xmlWriter.writeEndElement(); // Закрытие <member>
            }

        }
        movie.resetIterator();
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("release_date");
        xmlWriter.writeCharacters(movie.release_date);
        xmlWriter.writeEndElement(); // Закрытие release_date

        xmlWriter.writeStartElement("poster");
        xmlWriter.writeCharacters(movie.poster);
        xmlWriter.writeEndElement(); // Закрытие poster

        xmlWriter.writeEndDocument();

        file.close();
    }
    this->close();
    emit signalFinishRename("Movie", movie.id);
}

void DialogRenamerFiles::renameTV()
{
    QFileInfo oldPathToPoster(this->showTv.poster);
    QFileInfo oldPathShowTV(oldPathToPoster.absolutePath());

    QTreeWidgetItem *topLevelItem = ui->newListTV->topLevelItem(0);

    for (int i = 0; i < topLevelItem->childCount(); ++i) {
        QTreeWidgetItem *childItem = topLevelItem->child(i);

        QString newName = childItem->text(0);
        //Номер сезона
        uint season = childItem->data(1,Qt::UserRole).toUInt();
        //Номер эпизола
        uint episode = childItem->data(2,Qt::UserRole).toUInt();
        //id записи в БД
        int  id = childItem->data(3,Qt::UserRole).toInt();

        EpisodeInfo infoEpisode = this->showTv.getEpisode(season, episode);

        QString oldPath = infoEpisode.filePath;
        QString oldPoster = infoEpisode.still_path;

        QFileInfo oldPathInfo(oldPath);
        QFileInfo oldPosterInfo(oldPoster);

        QString newFileName;
        QString newFilePoster;
        QString rootPathShowTV = infoEpisode.libraryPath;

        QString seasonWithNull = QString::number(season, 10).rightJustified(2, '0');
        QString episodeWithNull = QString::number(episode, 10).rightJustified(2, '0');
        if(this->checkNewFoldersEpisodes){
            QString newPathToEpisodeShowTV = rootPathShowTV + "/Season "+seasonWithNull+"/"+newName+"."+oldPathInfo.suffix();
            newFileName = this->renameAndMoveFile(oldPath,newPathToEpisodeShowTV);
            if(oldPoster!=""){
                QString newPathToPosterEpisodeShowTV = rootPathShowTV + "/Season "+seasonWithNull+"/S"+seasonWithNull+"E"+episodeWithNull+" - "+ infoEpisode.episodeTitle +"-thumb."+oldPosterInfo.suffix();
                newFilePoster = this->renameAndMoveFile(oldPoster, newPathToPosterEpisodeShowTV);
            }

        }else{
            newFileName = this->renameFile(oldPath, newName);
            if(oldPoster!=""){
                newFilePoster = this->renameFile(oldPoster, newName);
            }
        }
        if(this->checkTVShowNFO){
            QFileInfo filepath(newFileName);
            QFile file(filepath.path()+"/"+newName+".nfo");

            if (!file.open(QIODevice::WriteOnly)) {
                qWarning() << tr("Не удалось открыть файл для записи:") << file.errorString();
                return;
            }
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);
            xmlWriter.writeStartDocument();

            xmlWriter.writeStartElement("episodedetails");

            xmlWriter.writeStartElement("title");
            xmlWriter.writeCharacters(infoEpisode.episodeTitle);
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("season");
            xmlWriter.writeCharacters(seasonWithNull);
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("episode");
            xmlWriter.writeCharacters(episodeWithNull);
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("aired");
            xmlWriter.writeCharacters(infoEpisode.air_date);
            xmlWriter.writeEndElement();

            xmlWriter.writeStartElement("plot");
            xmlWriter.writeCharacters(infoEpisode.overview);
            xmlWriter.writeEndElement();

            while(infoEpisode.nextCrew()){
                Crew existCrew = infoEpisode.getCrew();
                if(!existCrew.hasRole("Director")){
                    continue;
                }
                xmlWriter.writeStartElement("director");
                xmlWriter.writeCharacters(existCrew.name);
                xmlWriter.writeEndElement();
            }
            infoEpisode.resetIterator();
            while(infoEpisode.nextCrew()){
                Crew existCrew = infoEpisode.getCrew();
                if(!existCrew.hasRole("Writer")){
                    continue;
                }
                xmlWriter.writeStartElement("writer");
                xmlWriter.writeCharacters(existCrew.name);
                xmlWriter.writeEndElement();
            }
            infoEpisode.resetIterator();

            xmlWriter.writeStartElement("crew");

            while(infoEpisode.nextCrew()){
                Crew existCrew = infoEpisode.getCrew();
                for (const QString& roleName : existCrew.role) {
                    xmlWriter.writeStartElement("member");
                    xmlWriter.writeTextElement("name", existCrew.name);
                    xmlWriter.writeTextElement("role", roleName);
                    // xmlWriter.writeTextElement("department", "Writing");
                    xmlWriter.writeEndElement();
                }
            }
            infoEpisode.resetIterator();
            xmlWriter.writeEndElement();

            xmlWriter.writeEndDocument();

            file.close();
        }

        dbmanager->updateEpisodeColumn("file", newFileName, id);
        dbmanager->updateEpisodeColumn("Poster", newFilePoster, id);
    }

    QString pathtoTVShow = this->showTv.poster;
    if(this->checkTVShowNFO){
        QFileInfo filepath(pathtoTVShow);
        QFile file(filepath.path()+"/"+this->showTv.nameShow+".nfo");
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << tr("Не удалось открыть файл для записи:") << file.errorString();
            return;
        }
        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();

        // Начало корневого элемента
        xmlWriter.writeStartElement("tvshow");

        xmlWriter.writeStartElement("title");
        xmlWriter.writeCharacters(this->showTv.nameShow);
        xmlWriter.writeEndElement();


        xmlWriter.writeStartElement("originaltitle");
        xmlWriter.writeCharacters(this->showTv.originalNameShow);
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("year");
        xmlWriter.writeCharacters(this->showTv.first_air_date);
        xmlWriter.writeEndElement();

        QStringList listGenre = this->showTv.genres.split(",");
        for (const QString &genre : listGenre) {
            xmlWriter.writeStartElement("genre");
            xmlWriter.writeCharacters(genre);
            xmlWriter.writeEndElement(); // Закрытие director
        }

        QStringList production_companies = this->showTv.production_companies.split(",");
        for (const QString &comnpany : production_companies) {
            xmlWriter.writeStartElement("studio");
            xmlWriter.writeCharacters(comnpany);
            xmlWriter.writeEndElement();
        }

        xmlWriter.writeStartElement("plot");
        xmlWriter.writeCharacters(this->showTv.overview);
        xmlWriter.writeEndElement();

        xmlWriter.writeStartElement("crew");

        while(this->showTv.nextCrew()){
            Crew existCrew = this->showTv.getCrew();
            for (const QString& roleName : existCrew.role) {
                xmlWriter.writeStartElement("member");
                xmlWriter.writeTextElement("name", existCrew.name);
                xmlWriter.writeTextElement("role", roleName);
                // xmlWriter.writeTextElement("department", "Writing");
                xmlWriter.writeEndElement(); // Закрытие <member>
            }

        }
        this->showTv.resetIterator();
        xmlWriter.writeEndElement();

        xmlWriter.writeEndDocument();
    }
    this->close();
    emit signalFinishRename("TV", this->showTv.ID);
}


void DialogRenamerFiles::on_folderSeasonsCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    this->checkNewFoldersEpisodes = (arg1 == Qt::Checked);
}


void DialogRenamerFiles::on_createMovieNFOcheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    this->checkMovieNFO = (arg1==Qt::Checked);
}


void DialogRenamerFiles::on_createTVShowNFOcheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    this->checkTVShowNFO = (arg1==Qt::Checked);
}


void DialogRenamerFiles::on_renameClose_clicked()
{
    this->close();
}


void DialogRenamerFiles::on_renameButton_clicked()
{
    int index = ui->tabRenamer->currentIndex();
    switch (index) {
    case 0:
        this->renameMovie();
        break;
    case 1:
        this->renameTV();
        break;
    }
}

