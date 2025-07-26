#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QLocale>
#include <QTranslator>
#include "mainwindow.h"
bool setupApplicationDirectories() {
    QDir appDir;
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDir.exists(dataPath)) {
        if (!appDir.mkpath(dataPath)) {
            QMessageBox::critical(nullptr, "ENSIARY - Error", 
                                "Failed to create application data directory.\n" +
                                dataPath);
            return false;
        }
    }
    return true;
}
void configureApplicationStyle(QApplication& app) {
    app.setApplicationName("ENSIARY");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("ENSIARY - Library Management System");
    app.setOrganizationName("ENSIARY Development Team");
    app.setOrganizationDomain("ensiary.edu");
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::WindowText, QColor(30, 58, 138)); 
    lightPalette.setColor(QPalette::Base, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::AlternateBase, QColor(254, 243, 199)); 
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::ToolTipText, QColor(30, 58, 138));
    lightPalette.setColor(QPalette::Text, QColor(30, 58, 138)); 
    lightPalette.setColor(QPalette::Button, QColor(249, 115, 22)); 
    lightPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(249, 115, 22));
    lightPalette.setColor(QPalette::Highlight, QColor(249, 115, 22));
    lightPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    app.setPalette(lightPalette);
}
int main(int argc, char *argv[])
{    
    QApplication app(argc, argv);    
    app.setWindowIcon(QIcon(":/icon.png"));
    configureApplicationStyle(app);
    if (!setupApplicationDirectories()) {
        return -1;
    }
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ENSIARY_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }
    try {
        MainWindow window;
        window.show();
        return app.exec();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "ENSIARY - Critical Error", 
                            QString("An unexpected error occurred:\n%1\n\n"
                                   "The application will now exit.").arg(e.what()));
        return -1;
    }
    catch (...) {
        QMessageBox::critical(nullptr, "ENSIARY - Critical Error", 
                            "An unknown error occurred.\n"
                            "The application will now exit.");
        return -1;
    }
}