#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QDebug>
#include <QFont>
#include "src/ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("WiFiSettings");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("WiFiSettings");

    // Set default font
    QFont defaultFont = app.font();
    defaultFont.setFamily("Noto Sans CJK SC, Noto Sans, PingFang SC, Microsoft YaHei, sans-serif");
    defaultFont.setPointSize(10);
    app.setFont(defaultFont);

    // Load translations
    QTranslator translator;
    QString locale = QLocale::system().name(); // e.g., "zh_CN", "en_US"

    // Check command-line for language override: --lang zh or --lang en
    QStringList args = app.arguments();
    int langIdx = args.indexOf("--lang");
    if (langIdx >= 0 && langIdx + 1 < args.size()) {
        QString langArg = args[langIdx + 1];
        if (langArg.startsWith("zh")) {
            locale = "zh_CN";
        } else {
            locale = "en_US";
        }
    }

    // Determine translation file
    QString qmFile;
    if (locale.startsWith("zh")) {
        qmFile = "wifi_zh";
    } else {
        qmFile = "wifi_en";
    }

    // Try to load from Qt resource system
    if (translator.load(QString(":/translations/%1.qm").arg(qmFile))) {
        app.installTranslator(&translator);
        qDebug() << "Loaded translation:" << qmFile << "from resources";
    }
    // Fallback: try from filesystem
    else if (translator.load(qmFile, "resources/translations")) {
        app.installTranslator(&translator);
        qDebug() << "Loaded translation:" << qmFile << "from filesystem";
    } else {
        qDebug() << "Using default English (no translation loaded for" << locale << ")";
    }

    MainWindow window;
    window.show();

    return app.exec();
}
