#-------------------------------------------------
#
# Project created by QtCreator
#
#-------------------------------------------------

QT       += core gui qml declarative

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = xbmgr
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h


FORMS    +=

#######
#OTHER_FILES += \
#    qml/ToolBtn.qml \
#    qml/TabWidget.qml \
#    qml/SysBtnGroup.qml \
#    qml/SysBtn.qml \
#    qml/qmlmain.qml \
#    qml/MainWidget.qml \
#    qml/LabelBtn.qml \
#    qml/Content1.qml

RC_FILE += icon.rc

RESOURCES += \
    qml/default.qrc
