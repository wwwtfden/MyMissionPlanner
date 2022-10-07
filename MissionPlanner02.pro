QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cachepainter.cpp \
    clickablelabel.cpp \
    flightmapwidget.cpp \
    fmwdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    mousewheelwidgetadjustmentguard.cpp \
    waypoint.cpp \
    waypointwidget.cpp

HEADERS += \
    cachepainter.h \
    clickablelabel.h \
    flightmapwidget.h \
    fmwdialog.h \
    mainwindow.h \
    mousewheelwidgetadjustmentguard.h \
    waypoint.h \
    waypointwidget.h

FORMS += \
    mainwindow.ui
INCLUDEPATH += C:\libraries\opencv_2.4.13\build\include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
CONFIG(debug, debug|release) {
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_highgui2413d.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_imgproc2413d.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_core2413d.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_features2d2413d.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_nonfree2413d.lib
} else {
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_highgui2413.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_imgproc2413.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_core2413.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_features2d2413.lib
    LIBS += C:\libraries\opencv_2.4.13\build\x64\vc14\lib\opencv_nonfree2413.lib
}

RESOURCES += \
    MyRes.qrc

