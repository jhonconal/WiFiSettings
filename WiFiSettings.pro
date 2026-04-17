QT       += core gui widgets svg
CONFIG   += c++17

TARGET = WiFiSettings
TEMPLATE = app

# Directories
INCLUDEPATH += src/core src/ui

# Sources
SOURCES += \
    main.cpp \
    src/core/WifiNetwork.cpp \
    src/core/WifiBackend.cpp \
    src/core/WifiManager.cpp \
    src/ui/SwitchButton.cpp \
    src/ui/NetworkItemWidget.cpp \
    src/ui/NetworkListWidget.cpp \
    src/ui/PasswordDialog.cpp \
    src/ui/NetworkInfoDialog.cpp \
    src/ui/MainWindow.cpp

# Headers
HEADERS += \
    src/core/WifiNetwork.h \
    src/core/WifiBackend.h \
    src/core/WifiManager.h \
    src/ui/SwitchButton.h \
    src/ui/NetworkItemWidget.h \
    src/ui/NetworkListWidget.h \
    src/ui/PasswordDialog.h \
    src/ui/NetworkInfoDialog.h \
    src/ui/MainWindow.h

# Resources
RESOURCES += \
    resources/resources.qrc

# Translations
TRANSLATIONS += \
    resources/translations/wifi_en.ts \
    resources/translations/wifi_zh.ts

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
