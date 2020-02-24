QT       += core gui opengl concurrent xml charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
SOURCES += src/main.cpp  \
           src/MainWindow.cpp \
           src/MainWindow_context.cpp \
           src/MainWindow_actions.cpp \
           src/MainWindow_io.cpp \
           src/MainWindow_ui.cpp \
           src/MainWindow_functions.cpp \
           src/Viewer.cpp \
           src/ViewerWithMouse.cpp \
           src/VideoPlayerWidget.cpp \
           src/SeqInfoWidget.cpp \
           src/FrameSelectWidget.cpp \
           src/SegParameterWidget.cpp \
           src/TrackingParameterWidget.cpp \
           src/Header.cpp \
           src/iSeq.cpp \
           src/Image.cpp \
	   src/SegmentationWorker.cpp \
           src/MosquitoPosition.cpp \
           src/MosquitoesInFrame.cpp \ 
           src/MosquitoRegistry.cpp \
           src/TrackPoint.cpp \
           src/HistogramWidget.cpp \
           src/GraphWidget.cpp \
           src/MosquitoesInSeq.cpp \
           src/DisplayAllPointsWidget.cpp \
           src/MyVideoReader.cpp \
           src/Metadata.cpp \
           src/DisplaySettingsParameterWidget.cpp \
           src/ImageComposer.cpp
HEADERS  = src/MainWindow.h  \
           src/Viewer.h \
           src/ViewerWithMouse.h \
           src/VideoPlayerWidget.h \
           src/SeqInfoWidget.h \
           src/FrameSelectWidget.h \
           src/SegParameterWidget.h \
           src/TrackingParameterWidget.h \
           src/Header.hpp \
           src/iSeq.hpp \
           src/Image.hpp \
           src/SegmentationWorker.hpp \
           src/MosquitoPosition.hpp \
           src/MosquitoesInFrame.hpp \
           src/MosquitoRegistry.hpp \
           src/TrackPoint.hpp \
           src/HistogramWidget.h \
           src/GraphWidget.h \
           src/MosquitoesInSeq.hpp \
           src/DisplayAllPointsWidget.h \
           src/MyVideoReader.hpp \
           src/Metadata.hpp \
           src/DisplaySettingsParameterWidget.h \
           src/ImageComposer.hpp
#CONFIG += debug
CONFIG += console
CONFIG += c++11
CONFIG += link_pkgconfig
PKGCONFIG += opencv
PKGCONFIG += libavdevice
PKGCONFIG += libavformat
PKGCONFIG += libavfilter
PKGCONFIG += libavcodec
PKGCONFIG += libswresample
PKGCONFIG += libswscale
PKGCONFIG += libavutil
PKGCONFIG += icu-uc
PKGCONFIG += icu-io

QMAKE_CXXFLAGS += -fopenmp \
                  -D_LARGEFILE64_SOURCE \
                  -D_FILE_OFFSET_BITS=64 \
                  -D__STDC_FORMAT_MACROS

LIBS += -fopenmp

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_LFLAGS_RELEASE -= -O1

win32 {
RC_FILE = SeqFileProcessing2D.rc
}

RESOURCES     += SeqFileProcessing2D.qrc

#variables for about dialog
DEFINES += BUILDHOST='\\"$$system(hostname)\\"'
DEFINES += BUILDUSER='\\"$$system(echo $USER)\\"'
DEFINES += GITVERSION='\\"$$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags --long)\\"'

linux {
CONFIG += debug
iconFile.path = /usr/local/share/icons
iconFile.files = SeqFileProcessing2D.png
INSTALLS += iconFile

desktopFile.path = /usr/local/share/applications
desktopFile.files = SeqFileProcessing2D.desktop
INSTALLS += desktopFile

target.path = /usr/local/bin
INSTALLS += target
}
