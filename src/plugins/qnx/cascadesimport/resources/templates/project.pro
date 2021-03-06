#========================================================================================
# generated by CascadesProjectImporter ver. %IMPORTER_VERSION%  - %DATE_TIME%
#========================================================================================

TEMPLATE = app
TARGET = %TARGET%

CONFIG += %CONFIG%

# For cascades integration, some additional libraries are needed
# and the library path needs to be set
LIBS += -lbb -lbbcascades
LIBS += %EXTRA_LIBS%

QT -= gui
QT += %QT%

MOBILITY += %MOBILITY%

QMAKE_CFLAGS += -Wno-psabi
QMAKE_CXXFLAGS += -Wno-psabi

CONFIG(release, debug|release) {
    zygotize {
        TEMPLATE = lib
        QMAKE_CFLAGS += -fstack-protector-strong -fvisibility=hidden -mthumb -Os
        QMAKE_CXXFLAGS += -fstack-protector-strong -fvisibility=hidden -mthumb -Os
        QMAKE_LFLAGS += -Wl,-z,relro
        DEFINES += _FORTIFY_SOURCE=2
    }
}

INCLUDEPATH += src

HEADERS += \
    %HEADERS%

SOURCES += \
    %SOURCES%

RESOURCES += \
    %RESOURCES%

OTHER_FILES += \
    bar-descriptor.xml \
    %OTHER_FILES%
