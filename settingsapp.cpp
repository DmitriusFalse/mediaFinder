#include "settingsapp.h"
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
}

SettingsApp::~SettingsApp()
{

    delete ui;
}

void SettingsApp::saveLibraryFolder(){
    QList<libraryItem> libFolders;
    for (int row = 0; row < ui->tableDirsType->rowCount(); ++row) {
        // Получение значения из первой ячейки строки (например)
        QTableWidgetItem *itemPath = ui->tableDirsType->item(row, 0);
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableDirsType->cellWidget(row, 1));
        libFolders << libraryItem {itemPath->text(), comboBox->currentText()};
    }
    m_settings->writeLibraryToSettings(libFolders);
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
    qRegisterMetaType<libraryItem>("libraryItem");
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          "Выберите папку",
                                                          QDir::homePath(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!directory.isEmpty()) {
        QList<libraryItem> libFolders = m_settings->readLibraryFromSettings();
        libFolders << libraryItem{directory, "Movie"};
        libFolders = m_settings->checkLibraryDuplicate(libFolders);
        m_settings->writeLibraryToSettings(libFolders);

        SettingsApp::addPathToListLibrary();
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
    int row = ui->tableDirsType->currentRow();

    // Проверка, что строка действительно выделена (currentRow возвращает -1, если ничего не выделено)
    if (row >= 0) {
        // Удаление строки из таблицы
        ui->tableDirsType->removeRow(row);
    }
}

