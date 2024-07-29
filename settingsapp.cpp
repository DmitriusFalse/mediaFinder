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
#include <QTranslator>


//////////////////////////////////////////////////////////////////////////
/// \brief SettingsApp::SettingsApp
/// \param parent
//////////////////////////////////////////////////////////////////////////
SettingsApp::SettingsApp(QWidget *parent, DBManager *dbManager, SettingsData *settings, Settings *param)
    : QMainWindow(parent)
    , ui(new Ui::SettingsApp)
    , m_settings(settings)
    , m_dbManager(dbManager)
    , parametrs(param)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    SettingsApp::addPathToListLibrary();
    changeSettings = false;
    QString defLang = param->getSettings("language");
    if(defLang=="en-EN"){
        ui->defLangComboBox->setCurrentIndex(0);
    }else if(defLang=="ru-RU"){
        ui->defLangComboBox->setCurrentIndex(1);
    }else{
        ui->defLangComboBox->setCurrentIndex(0);
    }
    ui->tableDirsType->setColumnWidth(0, 400);
    ui->tableDirsType->setColumnWidth(1, 100);
    this->loadTranslation();

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

void SettingsApp::saveLangApp()
{
//settings

    switch (ui->defLangComboBox->currentIndex()) {
    case 0:{
        m_dbManager->saveSettings("language","en-EN");
    }
    break;
    case 1:{
        m_dbManager->saveSettings("language","ru-RU");
    }
    break;
default:
    break;
}

}

void SettingsApp::loadTranslation()
{
    this->parametrs = this->m_dbManager->getAllSettings();
    if(!this->translator.load(":/translation/"+this->parametrs->getSettings("language"))){
        qDebug() << "Error load translation";
    };
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);
}

void SettingsApp::closeEvent(QCloseEvent *event)
{
    if (changeSettings==false){
        event->accept();
    }else{
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Подтверждение"),
                                      tr("Сохранить изменения перед выходом?"),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            SettingsApp::saveLibraryFolder(true);
            event->accept(); // Разрешаем закрытие
        } else {
            event->accept(); // Отменяем закрытие
        }
    }
    emit signalWindowClosed();

}

void SettingsApp::on_saveButton_clicked()
{
    changeSettings = false;
    this->saveLangApp();
    this->loadTranslation();
    SettingsApp::saveLibraryFolder(true);
    close();
}

void SettingsApp::on_applySaveSettings_clicked()
{
    this->saveLangApp();
    this->loadTranslation();
    SettingsApp::saveLibraryFolder(false);
    emit signalApplySettings();
}

void SettingsApp::on_addPath_clicked()
{
    changeSettings = true;
    qRegisterMetaType<libraryItem>("libraryItem");
    QList<libraryItem> libFolders = m_settings->readLibraryFromSettings();
    QString directory = QFileDialog::getExistingDirectory(this,
                                                          tr("Выберите папку"),
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
        QMessageBox::information(nullptr, tr("Информация"),
                                 tr("Путь к библиотеке не добавлен.\n"
                                 "Путь '/home/user/media' является родительским\n"
                                 "для уже добавленного пути.\n"
                                 "Пожалуйста, выберите другую папку."));
    }
}

void SettingsApp::addPathToListLibrary(){

    qRegisterMetaType<libraryItem>("libraryItem");

    QList<libraryItem> libraryFolders = m_settings->readLibraryFromSettings();

    int size = libraryFolders.size();

    SettingsApp::ui->tableDirsType->setRowCount(size);
    SettingsApp::ui->tableDirsType->setColumnCount(2);
    SettingsApp::ui->tableDirsType->setHorizontalHeaderLabels(QStringList() << tr("Библиотека") << tr("Тип"));

    int row = 0; // номер строки, куда вы хотите вставить выпадающий список
    int column = 1; // номер столбца, куда вы хотите вставить выпадающий список
    int iconSize = 24;

    for (const auto &libFolder : libraryFolders) {
        QTableWidgetItem *itemPath = new QTableWidgetItem(libFolder.path);
        itemPath->setFlags(itemPath->flags() & ~Qt::ItemIsEditable);
        ui->tableDirsType->setItem(row, 0, itemPath);
        QComboBox *comboBox = new QComboBox();
        comboBox->setIconSize(QSize(iconSize, iconSize));
        QIcon iconMovie(":/icons/movie");
        QPixmap pixmapMovie = iconMovie.pixmap(iconSize, iconSize);

        comboBox->addItem(QIcon(pixmapMovie),"Movie");

        QIcon iconTV(":/icons/tv");
        QPixmap pixmapTV = iconTV.pixmap(iconSize, iconSize);
        comboBox->addItem(QIcon(pixmapTV),"TV");
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

