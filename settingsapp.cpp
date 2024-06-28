#include "settingsapp.h"
#include "qevent.h"
#include "qmessagebox.h"
#include "ui_settingsapp.h"
#include "settingsdata.h"
#include <QComboBox>
#include <QFileDialog>
#include <QVariant>
#include <QDataStream>
#include <QSettings>
#include <QMetaType>
#include <QString>


//////////////////////////////////////////////////////////////////////////
/// \brief SettingsApp::SettingsApp
/// \param parent
//////////////////////////////////////////////////////////////////////////
SettingsApp::SettingsApp(QWidget *parent, DBManager *dbManager, SettingsData *settings)
    : QMainWindow(parent)
    , ui(new Ui::SettingsApp)
    , m_settings(settings)
    , m_dbManager(dbManager)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    SettingsApp::addPathToListLibrary();
    changeSettings = false;
}

SettingsApp::~SettingsApp()
{

    delete ui;
}

void SettingsApp::saveLibraryFolder(bool update){
    QList<libraryItem> libFolders;
    for (int row = 0; row < ui->tableDirsType->rowCount(); ++row) {
        QTableWidgetItem *itemPath = ui->tableDirsType->item(row, 0);
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableDirsType->cellWidget(row, 1));
        libFolders << libraryItem {itemPath->text(), comboBox->currentText()};
    }
    m_settings->writeLibraryToSettings(libFolders);
    if(update){
        emit signalUpdateListCollection();
    }
}

void SettingsApp::closeEvent(QCloseEvent *event)
{
    if (changeSettings==false){
        qDebug() << changeSettings;
        event->accept();
    }else{
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Подтверждение",
                                      "Сохранить изменения перед выходом?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            SettingsApp::saveLibraryFolder(true);
            event->accept(); // Разрешаем закрытие
        } else {
            event->accept(); // Отменяем закрытие
        }
    }

}

void SettingsApp::on_saveButton_clicked()
{
    changeSettings = false;
    SettingsApp::saveLibraryFolder(true);
    close();
}

void SettingsApp::on_applySaveSettings_clicked()
{
    // changeSettings = true;
    SettingsApp::saveLibraryFolder(false);
}

void SettingsApp::on_addPath_clicked()
{
    changeSettings = true;
    qRegisterMetaType<libraryItem>("libraryItem");
    QList<libraryItem> libFolders = m_settings->readLibraryFromSettings();
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          "Выберите папку",
                                                          QDir::homePath(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    bool yesAdd = true;
    for (auto& libFolder : libFolders) {

        if(libFolder.path.startsWith (directory+"/")){
            yesAdd = false;
        }
    }
    if (yesAdd) {
        libFolders << libraryItem{directory, "Movie"};
        libFolders = m_settings->checkLibraryDuplicate(libFolders);
        m_settings->writeLibraryToSettings(libFolders);

        SettingsApp::addPathToListLibrary();
    }else{
        changeSettings = false;
        QMessageBox::information(nullptr, "Информация",
                                 "Путь к библиотеке не добавлен.\n"
                                 "Путь '/home/noc101/Медиа' является родительским\n"
                                 "для уже добавленного пути.\n"
                                 "Пожалуйста, выберите другую папку.");
    }
}

void SettingsApp::addPathToListLibrary(){

    qRegisterMetaType<libraryItem>("libraryItem");

    QList<libraryItem> libraryFolders = m_settings->readLibraryFromSettings();

    int size = libraryFolders.size();

    SettingsApp::ui->tableDirsType->setRowCount(size);
    SettingsApp::ui->tableDirsType->setColumnCount(2);
    SettingsApp::ui->tableDirsType->setHorizontalHeaderLabels(QStringList() << "Библиотека" << "Тип");

    int row = 0; // номер строки, куда вы хотите вставить выпадающий список
    int column = 1; // номер столбца, куда вы хотите вставить выпадающий список

    for (const auto &libFolder : libraryFolders) {
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

void SettingsApp::on_removeLibraryRow_clicked()
{
    changeSettings = true;
    int row = ui->tableDirsType->currentRow();
    if (row >= 0) {
        ui->tableDirsType->removeRow(row);
    }
}

