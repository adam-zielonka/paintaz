#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Adam Zielonka");
    QCoreApplication::setOrganizationDomain("adamzielonka.pro");
    QCoreApplication::setApplicationName("paintAZ");
#ifdef Q_OS_WASM
    QSettings::setDefaultFormat(QSettings::WebLocalStorageFormat);
#endif
    MainWindow w;
#ifdef Q_OS_WASM
    w.showFullScreen();
#else
    w.show();
#endif

    QTimer::singleShot(0, &w, &MainWindow::showWelcomeIfNeeded);
    return a.exec();
}
