#-------------------------------------------------
# Project file for a Qt Widgets application
#-------------------------------------------------

QT       += core gui widgets sql svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = ENSIARY

SOURCES += \
    DataBaseManager.cpp \
    main.cpp \
    mainwindow.cpp \
    book.cpp \
    user.cpp \
    librarymanager.cpp \
    student.cpp \
    admin.cpp

HEADERS += \
    DataBaseManager.h \
    mainwindow.h \
    book.h \
    user.h \
    librarymanager.h \
    student.h \
    admin.h 

RESOURCES += \
    resources.qrc 