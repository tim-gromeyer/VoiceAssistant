TARGET = markdownedit

QT       += core gui svg printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051208    # disables all the APIs deprecated before Qt 5.12.8

INCLUDEPATH += \
    src/

SOURCES += \
    src/common.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/about.cpp \
    src/highlighter.cpp \
    src/markdowneditor.cpp \
    src/markdownparser.cpp \
    src/spellchecker.cpp

HEADERS += \
    common.h \
    src/mainwindow.h \
    src/about.h \
    src/highlighter.h \
    src/markdowneditor.h \
    src/markdownparser.h \
    src/spellchecker.h

FORMS += \
    ui/mainwindow.ui

TRANSLATIONS += \
    translations/MarkdownEdit_de_DE.ts

include(3rdparty/qmarkdowntextedit/qmarkdowntextedit.pri)
include(3rdparty/md4c.pri)

CONFIG += lrelease
CONFIG += embed_translations
CONFIG -= qtquickcompiler

VERSION = 1.2.5
DEFINES += APP_VERSION=\\\"$$VERSION\\\" CHECK_MARKDOWN

android | wasm: DEFINES += NO_SPELLCHECK
else {
CONFIG += link_pkgconfig
PKGCONFIG += enchant-2
}

# Only show qDebug() messages in debug mode
CONFIG(release, debug | release): DEFINES += QT_NO_DEBUG_OUTPUT

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target

unix:!android {
desktop.path = /usr/share/applications
desktop.files = packaging/MarkdownEdit/usr/share/applications/MarkdownEdit.desktop
INSTALLS += desktop
}

RESOURCES += \
    ressources/icons.qrc \
    ressources/ressources.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/build.gradle \
    android/gradle.properties \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew \
    android/gradlew.bat \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/values/libs.xml \
    CMakeLists.txt \
    scripts/build.sh \
    README.md \
    packaging/build.sh

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
