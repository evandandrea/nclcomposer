#-------------------------------------------------
#
# Project created by QtCreator 2013-11-29T16:46:38
#
#-------------------------------------------------
TARGET = RulesView

include (../plugins-common.pri)

INCLUDEPATH += . \
            include

macx:LIBS +=    -L$$quote(/Library/Application Support/Composer/Extensions) \
                -lNCLLanguageProfile

win32:LIBS +=   -lNCLLanguageProfile

SOURCES += \
    src/rulesviewplugin.cpp \
    src/rulesviewfactory.cpp \
    src/rulesview.cpp \
    src/comboboxitem.cpp \
    src/ruleitem.cpp \
    src/compositeruleitem.cpp \
    src/rulestreewidget.cpp

HEADERS += \
    include/rulesviewplugin.h \
    include/rulesviewfactory.h \
    include/rulesview.h \
    include/comboboxitem.h \
    include/ruleitem.h \
    include/compositeruleitem.h \
    include/rulestreewidget.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
