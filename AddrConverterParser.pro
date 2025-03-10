QT       += core gui
QT += core5compat

greaterThan(QT_MAJOR_VERSION, 5.0): QT += widgets
CONFIG += c++11 console


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        modelfortable.cpp \
        notesfortable.cpp \
        processingfiles.cpp \
        widget.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    modelfortable.h \
    notesfortable.h \
    processingfiles.h \
    widget.h

FORMS += \
    widget.ui

include(logmanager/LogManager.pri)
include(SaveAll/saveall.pri)

DISTFILES +=
