!contains(DEFINES, __SAVEALL__) {
    DEFINES += __SAVEALL__

    SOURCES += \
        $$PWD/saveall.cpp
    HEADERS += \
        $$PWD/saveall.h
}
