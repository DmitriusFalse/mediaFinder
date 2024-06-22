#include "settingsapp.h"
#include "ui_settingsapp.h"
#include <QComboBox>
#include <QFileDialog>
#include <QVariant>
#include <QDataStream>
#include <QSettings>
#include <QMetaType>
#include <QString>

Q_DECLARE_METATYPE(libraryFolder)

QDataStream &operator<<(QDataStream &out, const libraryFolder &pathType)
{
    out << pathType.path << pathType.type;
    return out;
}

QDataStream &operator>>(QDataStream &in, libraryFolder &pathType)
{
    in >> pathType.path >> pathType.type;
    return in;
}
//////////////////////////////////////////////////////////////////////////
/// \brief SettingsApp::SettingsApp
/// \param parent
//////////////////////////////////////////////////////////////////////////
SettingsApp::SettingsApp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SettingsApp)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    qRegisterMetaType<libraryFolder>("libraryFolder");

    // w.setWindowTitle("MediaFinder");
    SettingsApp::addPathToListLibrary();
}

SettingsApp::~SettingsApp()
{
    delete ui;
}

void SettingsApp::saveLibraryFolder(){
    QList<libraryFolder> libFolders;
    for (int row = 0; row < ui->tableDirsType->rowCount(); ++row) {
        // Получение значения из первой ячейки строки (например)
        QTableWidgetItem *itemPath = ui->tableDirsType->item(row, 0);
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableDirsType->cellWidget(row, 1));
        libFolders << libraryFolder {itemPath->text(), comboBox->currentText()};
    }
    SettingsApp::writeStructToSettings(libFolders);
}

void SettingsApp::on_saveButton_clicked()
{
    SettingsApp::saveLibraryFolder();
    close();
}

void SettingsApp::on_applySaveSettings_clicked()
{
    SettingsApp::saveLibraryFolder();
}

void SettingsApp::on_addPath_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          "Выберите папку",
                                                          QDir::homePath(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!directory.isEmpty()) {
        QList<libraryFolder> libFolders = SettingsApp::readStructFromSettings();
        libFolders << libraryFolder{directory, "Movie"};
        libFolders = SettingsApp::checkDuplicate(libFolders);
        SettingsApp::writeStructToSettings(libFolders);

        SettingsApp::addPathToListLibrary();
    }
}

void SettingsApp::addPathToListLibrary(){

    QList<libraryFolder> libraryFolders = SettingsApp::readStructFromSettings();
    int size = libraryFolders.size();

    SettingsApp::ui->tableDirsType->setRowCount(size);
    SettingsApp::ui->tableDirsType->setColumnCount(2);
    SettingsApp::ui->tableDirsType->setHorizontalHeaderLabels(QStringList() << "Библиотека" << "Тип");

    int row = 0; // номер строки, куда вы хотите вставить выпадающий список
    int column = 1; // номер столбца, куда вы хотите вставить выпадающий список

    for (const auto &libFolder : libraryFolders) {
        qDebug() << "Path:" << libFolder.path << ", Type:" << libFolder.type;

        QTableWidgetItem *itemPath = new QTableWidgetItem(libFolder.path);
        itemPath->setFlags(itemPath->flags() & ~Qt::ItemIsEditable);
        ui->tableDirsType->setItem(row, 0, itemPath);
        QComboBox *comboBox = new QComboBox();
        comboBox->addItem("Movie");
        comboBox->addItem("TV");
        comboBox->setCurrentText(libFolder.type);
        SettingsApp::ui->tableDirsType->setCellWidget(row, column, comboBox);
        row++;
    }
    ui->tableDirsType->resizeColumnsToContents();

}
///
/// \brief SettingsApp::writeStructToSettings
/// \param data
///
void SettingsApp::writeStructToSettings(const QList<libraryFolder> &data)
{
        QSettings settings("MediaFinder", "settings");

        settings.beginWriteArray("library", data.size());
        for (int i = 0; i < data.size(); ++i) {
            if (data.at(i).path != "" and data.at(i).type != ""){
                settings.setArrayIndex(i);
                settings.setValue("libraryFolder"+ QString::number(i), QVariant::fromValue(data.at(i)));
            }

        }
        settings.endArray();
}
///
/// \brief readStructFromSettings
/// \return
///
QList<libraryFolder> SettingsApp::readStructFromSettings()
{
        QSettings settings("MediaFinder", "settings");

        int size = settings.beginReadArray("library");
        QList<libraryFolder> data;
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            QVariant variant = settings.value("libraryFolder"+ QString::number(i));
            if (variant.canConvert<libraryFolder>()) {
                libraryFolder libFolder = variant.value<libraryFolder>();
                data.append(libFolder);
            }
        }
        settings.endArray();

        return data;
}

QList<libraryFolder> SettingsApp::checkDuplicate(QList<libraryFolder> libFolder) {
        // QList<libraryFolder> newLibFolders;
        // for (const auto &itemFolder : libFolder) {
        //     bool insert = true;
        //     for (const auto &newItemFolder : newLibFolders) {
        //         if(itemFolder.path==newItemFolder.path){
        //             insert = false;
        //             break;
        //         }
        //     }
        //     if (insert == true){
        //         newLibFolders << itemFolder;
        //     }
        // }
        // return newLibFolders;
    return libFolder;
}

void SettingsApp::on_pushButton_clicked()
{
    int row = ui->tableDirsType->currentRow();

    // Проверка, что строка действительно выделена (currentRow возвращает -1, если ничего не выделено)
    if (row >= 0) {
        // Удаление строки из таблицы
        ui->tableDirsType->removeRow(row);
    }
}



