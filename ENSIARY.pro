QT += core widgets
CONFIG += c++20
TARGET = ENSIARY
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/models/resource.cpp \
    src/models/book.cpp \
    src/models/article.cpp \
    src/models/thesis.cpp \
    src/models/digitalcontent.cpp \
    src/models/user.cpp \
    src/models/loan.cpp \
    src/models/reservation.cpp \
    src/services/library_manager.cpp \
    src/services/persistence_service.cpp \
    src/dialogs/resource_dialog.cpp \
    src/dialogs/user_dialog.cpp \
    src/dialogs/user_loans_dialog.cpp \
    src/dialogs/reservation_management_dialog.cpp
HEADERS += \
    src/mainwindow.h \
    src/models/resource.h \
    src/models/book.h \
    src/models/article.h \
    src/models/thesis.h \
    src/models/digitalcontent.h \
    src/models/user.h \
    src/models/loan.h \
    src/models/reservation.h \
    src/services/library_manager.h \
    src/services/persistence_service.h \
    src/dialogs/resource_dialog.h \
    src/dialogs/user_dialog.h \
    src/dialogs/user_loans_dialog.h \
    src/dialogs/reservation_management_dialog.h
FORMS += \
    src/mainwindow.ui
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
# Include paths
INCLUDEPATH += src/models src/services src/dialogs
QMAKE_CXXFLAGS += -std=c++20
VERSION = 1.0.0
TARGET = ENSIARY
DEFINES += APP_NAME=\\\"ENSIARY\\\"
DEFINES += APP_VERSION=\\\"1.0.0\\\"
RESOURCES += resources/resources.qrc
unix:!macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    target.path = $$PREFIX/bin
    shortcutfiles.files = resources/ENSIARY.desktop
    shortcutfiles.path = $$PREFIX/share/applications/
    pixmapfiles.files = resources/icon.png
    pixmapfiles.path = $$PREFIX/share/pixmaps/
    INSTALLS += shortcutfiles
    INSTALLS += pixmapfiles
    INSTALLS += target
}