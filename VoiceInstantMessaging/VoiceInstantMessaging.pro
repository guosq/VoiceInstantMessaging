QT       += core gui network widgets multimedia concurrent
android{
    QT+= androidextras
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
win* {
    DESTDIR = $$_PRO_FILE_PWD_/../bin/
}

CONFIG += c++11
CONFIG += debug_and_release

CONFIG(debug, debug|release) {
    TARGET = VoiceInstantMessagingD

    win* {
        contains(QMAKE_HOST.arch, x86_64){
            message("编译平台：windows x86_64")
        }else{
            message("编译平台：windows x86")
        }

        INCLUDEPATH += ../thirdLibs/win32/include

        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavformat-58
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavdevice-58
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavcodec-58
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavutil-56
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lswscale-5
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lswresample-3
    }

    linux* {
        contains(QMAKE_HOST.arch, x86_64){
            message("编译平台：linux x86_64")
            INCLUDEPATH += ../thirdLibs/linux/include

            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavformat
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavdevice
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavcodec
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavutil
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lswscale
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lswresample
        }else{
            message("编译平台：linux arm64")
            INCLUDEPATH += ../thirdLibs/include
        }
    }

} else {
    TARGET = VoiceInstantMessaging

    win* {
        contains(QMAKE_HOST.arch, x86_64){
            message("编译平台：windows x86_64")
        }else{
            message("编译平台：windows x86")
        }

        INCLUDEPATH += ../thirdLibs/win32/include

        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavformat-58
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavdevice-58
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavcodec-58
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lavutil-56
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lswscale-5
        LIBS += -L$$_PRO_FILE_PWD_/../bin/ -lswresample-3
    }
    linux* {
        contains(QMAKE_HOST.arch, x86_64){
            message("编译平台：linux x86_64")
            INCLUDEPATH += ../thirdLibs/linux/include

            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavformat
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavdevice
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavcodec
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lavutil
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lswscale
            LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/linux/ -lswresample
        }else{
            message("编译平台：linux arm64")
            INCLUDEPATH += ../thirdLibs/include
        }

    }
}

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
    AudioIODevice.cpp \
    AudioPlayForm.cpp \
    AudioRecordForm.cpp \
    main.cpp \
    MainWindow.cpp \
    AudioEncoder.cpp \
    AudioDecoder.cpp

HEADERS += \
    AudioIODevice.h \
    AudioPlayForm.h \
    AudioRecordForm.h \
    MainWindow.h \
    AudioEncoder.h \
    AudioDecoder.h

FORMS += \
    AudioPlayForm.ui \
    AudioRecordForm.ui \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

android{
    langs.files += ./*.qm
    langs.path = /assets/dict
    INSTALLS += langs

    data.files += ./data/*.*
    data.path = /assets/
    INSTALLS += data
}


contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android


    ANDROID_EXTRA_LIBS = \
        $$PWD/../thirdLibs/android-armeabi-v7a/libavcodec.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libavdevice.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libavfilter.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libavformat.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libavutil.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libcrypto_1_1.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libssl_1_1.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libswresample.so \
        $$PWD/../thirdLibs/android-armeabi-v7a/libswscale.so

    LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/android-armeabi-v7a/ -lavformat -lavdevice -lavcodec -lavutil -lswscale -lswresample

}

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android


    ANDROID_EXTRA_LIBS = \
        $$PWD/../thirdLibs/android-arm64-v8a/libavcodec.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libavdevice.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libavfilter.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libavformat.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libavutil.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libcrypto_1_1.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libssl_1_1.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libswresample.so \
        $$PWD/../thirdLibs/android-arm64-v8a/libswscale.so

    LIBS += -L$$_PRO_FILE_PWD_/../thirdLibs/android-arm64-v8a/ -lavformat -lavdevice -lavcodec -lavutil -lswscale -lswresample
}


contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android


    ANDROID_EXTRA_LIBS = \
        $$PWD/../thirdLibs/android-x86/libavcodec.so \
        $$PWD/../thirdLibs/android-x86/libavdevice.so \
        $$PWD/../thirdLibs/android-x86/libavfilter.so \
        $$PWD/../thirdLibs/android-x86/libavformat.so \
        $$PWD/../thirdLibs/android-x86/libavutil.so \
        $$PWD/../thirdLibs/android-x86/libcrypto_1_1.so \
        $$PWD/../thirdLibs/android-x86/libssl_1_1.so \
        $$PWD/../thirdLibs/android-x86/libswresample.so \
        $$PWD/../thirdLibs/android-x86/libswscale.so
}

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml
