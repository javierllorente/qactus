# -------------------------------------------------
# Project created by QtCreator 2010-12-22T04:00:42
# -------------------------------------------------
unix:isEmpty(PREFIX):PREFIX = /usr/local
QT += network
TARGET = qactus
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR += .
SOURCES += main.cpp \
    mainwindow.cpp \
    trayicon.cpp \
    configure.cpp \
    login.cpp \
    obspackage.cpp \
    obsaccess.cpp \
    obsxmlreader.cpp \
    obsrequest.cpp \
    roweditor.cpp
HEADERS += mainwindow.h \
    trayicon.h \
    configure.h \
    login.h \
    obspackage.h \
    obsaccess.h \
    obsxmlreader.h \
    obsrequest.h \
    roweditor.h
FORMS += mainwindow.ui \
    configure.ui \
    login.ui \
    roweditor.ui
OTHER_FILES += README.md \
    LICENSE \
    license_template.txt
INSTALLS += target \
    icon128 \
    desktop
target.path = $$PREFIX/bin
desktop.path = $$PREFIX/share/applications/
desktop.files += qactus.desktop
icon128.path = $$PREFIX/share/icons/hicolor/128x128/apps
icon128.files += icons/128x128/qactus.png
RESOURCES = application.qrc
