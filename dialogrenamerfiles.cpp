#include "dialogrenamerfiles.h"
#include "dbmanager.h"
#include "ui_dialogrenamerfiles.h"
#include <QFileInfo>
#include <QRegularExpression>


DialogRenamerFiles::DialogRenamerFiles(QWidget *parent, DBManager *db, MovieInfo *m)
    : QDialog(parent)
    , ui(new Ui::DialogRenamerFiles)
    , dbmanager(db)
    , movie(m)
{
    ui->setupUi(this);
// oldListMovie
    this->replacePlaceholders = new Placeholders(*movie);
    QFileInfo fileinfo(movie->path);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->oldListMovie);
    newItem->setText(0, fileinfo.fileName());
    ui->patternMovieEdit->setText(fileinfo.fileName());

    this->oldIitem = new QTreeWidgetItem(ui->newListMovie);
    QString description = "<table>";

    while (this->replacePlaceholders->next()) {
        QString key = this->replacePlaceholders->currentKey();
        QString desc = this->replacePlaceholders->getFieldDescription(key);
        QString value = this->replacePlaceholders->getValue(key);

        // Формируем строку для каждого элемента таблицы
        QString row = "<tr>"
                      "<td><b>Описание:</b> " + desc + "</td>"
                               "<td><b>Подстановочное слово:</b> " + key + "</td>"
                              "<td><b>Пример:</b> " + value + "</td>"
                                "</tr>";

        // Добавляем сформированную строку в общий текст таблицы
        description += row;
    }

    description += "</table>";
    ui->patternDescription->setHtml(description);
}

DialogRenamerFiles::~DialogRenamerFiles()
{
    delete ui;
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

