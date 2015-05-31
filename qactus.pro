# -------------------------------------------------
# Project created by QtCreator 2010-12-22T04:00:42
# -------------------------------------------------
unix:isEmpty(PREFIX):PREFIX = /usr/local
unix {
    INCLUDEPATH += /usr/include/qtkeychain
    contains(QMAKE_HOST.arch, x86_64) {
        LIBS += -L/usr/lib64/ -lqtkeychain
    } else {
        LIBS += -L/usr/lib/ -lqtkeychain
    }
}
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
    roweditor.cpp \
    requeststateeditor.cpp \
    autotooltipdelegate.cpp
HEADERS += mainwindow.h \
    trayicon.h \
    configure.h \
    login.h \
    obspackage.h \
    obsaccess.h \
    obsxmlreader.h \
    obsrequest.h \
    roweditor.h \
    requeststateeditor.h \
    autotooltipdelegate.h
FORMS += mainwindow.ui \
    configure.ui \
    login.ui \
    roweditor.ui \
    requeststateeditor.ui
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
icon128.files += qactus.png
RESOURCES = application.qrc
