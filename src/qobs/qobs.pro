QT += core network
TARGET = qobs
TEMPLATE = lib
DEFINES += QOBS_LIBRARY
include(../defines.pri)

SOURCES += obspackage.cpp \
    obsaccess.cpp \
    obsxmlreader.cpp \
    obsrequest.cpp \
    obs.cpp \
    obsfile.cpp \
    obsresult.cpp \
    obsrevision.cpp \
    obsstatus.cpp \
    obsabout.cpp
HEADERS += obspackage.h \
    obsaccess.h \
    obsxmlreader.h \
    obsrequest.h \
    obs.h \
    obsfile.h \
    obsresult.h \
    obsrevision.h \
    obsstatus.h \
    obsabout.h

unix {
    contains(QMAKE_HOST.arch, x86_64) {
        target.path = $$PREFIX/lib64
    } else {
        target.path = $$PREFIX/lib
    }
}
INSTALLS += target

