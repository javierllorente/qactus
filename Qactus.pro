# -------------------------------------------------
# Project created by QtCreator 2010-12-22T04:00:42
# -------------------------------------------------
QT += network
TARGET = Qactus
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    trayicon.cpp \
    configure.cpp \
    login.cpp \
    obspackage.cpp \
    obsaccess.cpp \
    obsxmlreader.cpp \
    obsrequest.cpp
HEADERS += mainwindow.h \
    trayicon.h \
    configure.h \
    login.h \
    obspackage.h \
    obsaccess.h \
    obsxmlreader.h \
    obsrequest.h
FORMS += mainwindow.ui \
    configure.ui \
    login.ui
OTHER_FILES += README.md \
    LICENSE \
    license_template.txt
RESOURCES = application.qrc
