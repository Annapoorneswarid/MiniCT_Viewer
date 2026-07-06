QT       += core gui widgets

TARGET = MiniCTViewer
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    imageviewer.cpp

HEADERS += \
    mainwindow.h \
    imageviewer.h

FORMS += \
    mainwindow.ui
