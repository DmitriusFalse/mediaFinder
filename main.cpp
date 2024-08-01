#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("libs");
    QCoreApplication::setLibraryPaths(paths);

    a.setApplicationName("MediaFinder");
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MediaFinder_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;

    w.setWindowTitle("MediaFinder");
    w.show();
    return a.exec();
}
