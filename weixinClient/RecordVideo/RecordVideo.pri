
QT       += core gui

INCLUDEPATH+=D:\opencv-release\include\opencv\
                D:\opencv-release\include\opencv2\
                D:\opencv-release\include

LIBS+=D:\opencv-release\lib\libopencv_*.dll.a


HEADERS += \
    $$PWD/screen_read.h \
    $$PWD/video_read.h \
    $$PWD/common.h \
    $$PWD/myfacedetect.h

SOURCES += \
    $$PWD/screen_read.cpp \
    $$PWD/video_read.cpp \
    $$PWD/myfacedetect.cpp


#D:\opencv-release\lib\libopencv_calib3d2410.dll.a\
#       D:\opencv-release\lib\libopencv_contrib2410.dll.a\
#       D:\opencv-release\lib\libopencv_core2410.dll.a\
#       D:\opencv-release\lib\libopencv_features2d2410.dll.a\
#       D:\opencv-release\lib\libopencv_flann2410.dll.a\
#       D:\opencv-release\lib\libopencv_gpu2410.dll.a\
#       D:\opencv-release\lib\libopencv_highgui2410.dll.a\
#       D:\opencv-release\lib\libopencv_imgproc2410.dll.a\
#       D:\opencv-release\lib\libopencv_legacy2410.dll.a\
#       D:\opencv-release\lib\libopencv_ml2410.dll.a\
#       D:\opencv-release\lib\libopencv_objdetect2410.dll.a\
#       D:\opencv-release\lib\libopencv_video2410.dll.a\
