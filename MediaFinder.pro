QT       += core gui network sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT_MIN_VERSION = 6.0.0
QT_MAJOR_VERSION = 6
CONFIG += c++17
CONFIG += release

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dbmanager.cpp \
    dialogrenamerfiles.cpp \
    dialogshowprogress.cpp \
    main.cpp \
    mainwindow.cpp \
    medialibrary.cpp \
    searchmedia.cpp \
    secretVault.cpp \
    settingsapp.cpp \
    settingsdata.cpp \
    showimagefile.cpp \
    workrefreshmovie.cpp

HEADERS += \
    GenreList.h \
    LibraryItem.h \
    dbmanager.h \
    dialogrenamerfiles.h \
    dialogshowprogress.h \
    mainwindow.h \
    medialibrary.h \
    movieCollections.h \
    searchmedia.h \
    secretVault.h \
    settingsapp.h \
    settingsdata.h \
    showimagefile.h \
    workrefreshmovie.h

FORMS += \
    dialogrenamerfiles.ui \
    dialogshowprogress.ui \
    mainwindow.ui \
    searchmedia.ui \
    settingsapp.ui \
    showimagefile.ui

TRANSLATIONS += \
    MediaFinder_ru_RU.ts \  
    MediaFinder_en_EN.ts \
    MediaFinder_en_EN.qm \
    MediaFinder_ru_RU.qm \
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  data.qrc
