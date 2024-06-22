/********************************************************************************
** Form generated from reading UI file 'settingsapp.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSAPP_H
#define UI_SETTINGSAPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsApp
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_2;
    QVBoxLayout *topLayout;
    QTableWidget *tableDirsType;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *addPath;
    QPushButton *pushButton;
    QHBoxLayout *bottomLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *applySaveSettings;
    QPushButton *saveButton;
    QSpacerItem *verticalSpacer;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *SettingsApp)
    {
        if (SettingsApp->objectName().isEmpty())
            SettingsApp->setObjectName("SettingsApp");
        SettingsApp->resize(909, 781);
        centralwidget = new QWidget(SettingsApp);
        centralwidget->setObjectName("centralwidget");
        gridLayout_2 = new QGridLayout(centralwidget);
        gridLayout_2->setObjectName("gridLayout_2");
        topLayout = new QVBoxLayout();
        topLayout->setObjectName("topLayout");
        tableDirsType = new QTableWidget(centralwidget);
        if (tableDirsType->columnCount() < 2)
            tableDirsType->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableDirsType->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableDirsType->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        tableDirsType->setObjectName("tableDirsType");
        tableDirsType->setEnabled(true);
        tableDirsType->setAutoFillBackground(true);
        tableDirsType->setAlternatingRowColors(true);
        tableDirsType->horizontalHeader()->setCascadingSectionResizes(false);

        topLayout->addWidget(tableDirsType);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        addPath = new QPushButton(centralwidget);
        addPath->setObjectName("addPath");

        horizontalLayout->addWidget(addPath);

        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName("pushButton");

        horizontalLayout->addWidget(pushButton);


        topLayout->addLayout(horizontalLayout);


        gridLayout_2->addLayout(topLayout, 0, 0, 1, 1);

        bottomLayout = new QHBoxLayout();
        bottomLayout->setObjectName("bottomLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        bottomLayout->addItem(horizontalSpacer);

        applySaveSettings = new QPushButton(centralwidget);
        applySaveSettings->setObjectName("applySaveSettings");

        bottomLayout->addWidget(applySaveSettings);

        saveButton = new QPushButton(centralwidget);
        saveButton->setObjectName("saveButton");

        bottomLayout->addWidget(saveButton);


        gridLayout_2->addLayout(bottomLayout, 2, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 1, 0, 1, 1);

        SettingsApp->setCentralWidget(centralwidget);
        menubar = new QMenuBar(SettingsApp);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 909, 23));
        SettingsApp->setMenuBar(menubar);
        statusbar = new QStatusBar(SettingsApp);
        statusbar->setObjectName("statusbar");
        SettingsApp->setStatusBar(statusbar);

        retranslateUi(SettingsApp);

        QMetaObject::connectSlotsByName(SettingsApp);
    } // setupUi

    void retranslateUi(QMainWindow *SettingsApp)
    {
        SettingsApp->setWindowTitle(QCoreApplication::translate("SettingsApp", "MainWindow", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableDirsType->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("SettingsApp", "Path", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableDirsType->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("SettingsApp", "Type", nullptr));
        addPath->setText(QCoreApplication::translate("SettingsApp", "+ Add", nullptr));
        pushButton->setText(QCoreApplication::translate("SettingsApp", "- Remove", nullptr));
        applySaveSettings->setText(QCoreApplication::translate("SettingsApp", "\320\237\321\200\320\270\320\274\320\265\320\275\320\270\321\202\321\214", nullptr));
        saveButton->setText(QCoreApplication::translate("SettingsApp", "\320\241\320\276\321\205\321\200\320\260\320\275\320\270\321\202\321\214 \320\270 \320\262\321\213\320\271\321\202\320\270", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsApp: public Ui_SettingsApp {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSAPP_H
