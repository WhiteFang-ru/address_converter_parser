!contains(DEFINES, __LOGMANAGER__) {
DEFINES += __LOGMANAGER__
HEADERS += \
    $$PWD/logmanager.h

SOURCES += \
    $$PWD/logmanager.cpp
}
