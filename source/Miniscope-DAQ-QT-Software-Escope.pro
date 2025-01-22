QT += qml quick widgets gui core
CONFIG += c++11

#QT += 3dcore

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += DEBUG
#DEFINES += USE_USB
DEFINES += USE_PYTHON

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        backend.cpp \
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

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = .  # absolute path: [PROJECT_FOLDER_DIRECTORY]/source

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Add Icon
RC_ICONS = miniscope_icon.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    backend.h \
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

DISTFILES += \
    ../Scripts/DLCwrapper.py \
    ../deviceConfigs/behaviorCams.json \
    ../deviceConfigs/miniscopes.json \
    ../deviceConfigs/userConfigProps.json \
    ../deviceConfigs/videoDevices.json

win32 {
    # Path to your openCV .lib file(s)
    LIBS += -LC:/opencv4.10.0_MinGW\install\x64\mingw\bin -lopencv_core4100d -lopencv_highgui4100d -lopencv_imgproc4100d -lopencv_imgcodecs4100d -lopencv_videoio4100d

#    LIBS += -LC:/opencv-4.4.0/build/lib/Debug -lopencv_world440d

    # Path to openCV header files
    INCLUDEPATH += C:/opencv4.10.0_MinGW/install/include

    # Do we need DEPENDPATH ????
#    #DEPENDPATH +=

    # For libusb
#    LIBS += -LC:/libusb-1.0.24/VS2017/MS64/dll/ -llibusb-1.0
#    INCLUDEPATH += C:/libusb-1.0.24/include/libusb-1.0

#    LIBS += -LC:/libusb-1.0.23/MS64/dll/ -llibusb-1.0
#    INCLUDEPATH += C:/libusb-1.0.23/include/libusb-1.0

    # For Python
    INCLUDEPATH += C:/Python/Python311/include
    LIBS += -LC:/Python/Python311/libs -lpython311
    #C:\Users\LAb\AppData\Local\Programs\Python\Python311
#    INCLUDEPATH += C:/Users/dbaha/.conda/envs/basepy37/include
#    LIBS += -LC:/Users/dbaha/.conda/envs/basepy37/libs -lpython37

    # For numpy
    INCLUDEPATH += C:/Python/Python311/Lib/site-packages/numpy
    INCLUDEPATH += C:/Python/Python311/Lib/site-packages/numpy/_core/include
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4
}

# Move user and device configs to build directory
copydata.commands = $(COPY_DIR) \"$$shell_path($$PWD\\..\\deviceConfigs)\" \"$$shell_path($$OUT_PWD\\release\\deviceConfigs)\"
copydata2.commands = $(COPY_DIR) \"$$shell_path($$PWD\\..\\userConfigs)\" \"$$shell_path($$OUT_PWD\\release\\userConfigs)\"
copydata3.commands = $(COPY_DIR) \"$$shell_path($$PWD\\..\\Scripts)\" \"$$shell_path($$OUT_PWD\\release\\Scripts)\"
first.depends = $(first) copydata copydata2 copydata3
export(first.depends)
export(copydata.commands)
export(copydata2.commands)
export(copydata3.commands)

QMAKE_EXTRA_TARGETS += first copydata copydata2 copydata3
