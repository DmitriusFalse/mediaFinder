/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QGridLayout *gridLayout_2;
    QTabWidget *tabWidget;
    QWidget *tabMovie;
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_2;
    QTreeWidget *listMovieLibrary;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLineEdit *lineEdit;
    QPlainTextEdit *plainTextEdit;
    QSpacerItem *verticalSpacer;
    QWidget *tabTV;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *pushButton;
    QPushButton *refreshLibrary;
    QSpacerItem *horizontalSpacer;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1191, 922);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName("gridLayout_2");
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setTabPosition(QTabWidget::West);
        tabMovie = new QWidget();
        tabMovie->setObjectName("tabMovie");
        horizontalLayout = new QHBoxLayout(tabMovie);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        listMovieLibrary = new QTreeWidget(tabMovie);
        listMovieLibrary->setObjectName("listMovieLibrary");

        horizontalLayout_2->addWidget(listMovieLibrary);


        horizontalLayout->addLayout(horizontalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label = new QLabel(tabMovie);
        label->setObjectName("label");

        horizontalLayout_3->addWidget(label);

        lineEdit = new QLineEdit(tabMovie);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setEnabled(false);

        horizontalLayout_3->addWidget(lineEdit);


        verticalLayout->addLayout(horizontalLayout_3);

        plainTextEdit = new QPlainTextEdit(tabMovie);
        plainTextEdit->setObjectName("plainTextEdit");
        plainTextEdit->setMaximumSize(QSize(16777215, 150));

        verticalLayout->addWidget(plainTextEdit);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        horizontalLayout->addLayout(verticalLayout);

        tabWidget->addTab(tabMovie, QString());
        tabTV = new QWidget();
        tabTV->setObjectName("tabTV");
        tabWidget->addTab(tabTV, QString());

        gridLayout_2->addWidget(tabWidget, 0, 0, 1, 1);


        gridLayout->addLayout(gridLayout_2, 1, 0, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName("pushButton");

        horizontalLayout_5->addWidget(pushButton);

        refreshLibrary = new QPushButton(centralwidget);
        refreshLibrary->setObjectName("refreshLibrary");

        horizontalLayout_5->addWidget(refreshLibrary);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer);


        gridLayout->addLayout(horizontalLayout_5, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1191, 23));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\320\235\320\260\320\267\320\262\320\260\320\275\320\270\320\265", nullptr));
        lineEdit->setText(QCoreApplication::translate("MainWindow", "\320\237\321\200\320\270\320\274\320\265\321\200", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabMovie), QCoreApplication::translate("MainWindow", "Movie", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabTV), QCoreApplication::translate("MainWindow", "TV", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "\320\235\320\260\321\201\321\202\321\200\320\276\320\271\320\272\320\270", nullptr));
        refreshLibrary->setText(QCoreApplication::translate("MainWindow", "\320\236\320\261\320\275\320\276\320\262\320\270\321\202\321\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
