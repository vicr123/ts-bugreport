#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <iostream>

void QtHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        std::cerr << msg.toStdString() + "\n";
        break;
    case QtFatalMsg:
        std::cerr << msg.toStdString() + "\n";
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(QtHandler);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationName("ts-bugreport");

    MainWindow w;
    w.show();

    return a.exec();
}
