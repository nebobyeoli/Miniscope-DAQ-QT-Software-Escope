######################################################################
# Automatically generated by qmake (3.1) Wed Jul 12 20:07:50 2023
######################################################################

TEMPLATE = app
TARGET = source
INCLUDEPATH += .

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += backend.h \
           behaviorcam.h \
           behaviortracker.h \
           behaviortrackerworker.h \
           controlpanel.h \
           datasaver.h \
           miniscope.h \
           newquickview.h \
           tracedisplay.h \
           videodevice.h \
           videodisplay.h \
           videostreamocv.h
SOURCES += backend.cpp \
           behaviorcam.cpp \
           behaviortracker.cpp \
           behaviortrackerworker.cpp \
           controlpanel.cpp \
           datasaver.cpp \
           main.cpp \
           miniscope.cpp \
           newquickview.cpp \
           tracedisplay.cpp \
           videodevice.cpp \
           videodisplay.cpp \
           videostreamocv.cpp
RESOURCES += qml.qrc
