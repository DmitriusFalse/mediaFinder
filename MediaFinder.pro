QT       += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dbmanager.cpp \
    dialogshowprogress.cpp \
    main.cpp \
    mainwindow.cpp \
    medialibrary.cpp \
    searchmedia.cpp \
    settingsapp.cpp \
    settingsdata.cpp \
    workrefreshmovie.cpp

HEADERS += \
    GenreList.h \
    LibraryItem.h \
    dbmanager.h \
    dialogshowprogress.h \
    mainwindow.h \
    medialibrary.h \
    movieCollections.h \
    searchmedia.h \
    settingsapp.h \
    settingsdata.h \
    workrefreshmovie.h

FORMS += \
    dialogshowprogress.ui \
    mainwindow.ui \
    searchmedia.ui \
    settingsapp.ui

TRANSLATIONS += \
    MediaFinder_ru_RU.ts
CONFIG += lrelease
CONFIG += embed_translations



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
