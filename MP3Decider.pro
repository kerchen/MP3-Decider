CONFIG += qt
CONFIG += uitools debug_and_release
QT += webkit
QT += phonon
UI_HEADERS_DIR = ./ui_inc
MOC_DIR = ./moc
RCC_DIR = ./qrc
OBJECTS_DIR = ./release
JAM_BUILD_TYPE = release
DESTDIR = ./release
CONFIG(debug,debug|release) {
	OBJECTS_DIR = ./debug
	DESTDIR = ./debug
}
TARGET = MP3Decider

win32 {
	SLASH = \\
}
linux-g++ {
	SLASH = /
}

INCLUDEPATH += src
INCLUDEPATH += $${UI_HEADERS_DIR}


TEMPLATE = app

FORMS = ui/ConfigureDlg.ui \
		ui/MainWindow.ui 
						
RESOURCES = ui/res/MP3Decider.qrc

SOURCES += \
			src/MP3DConfigureDlg.cpp \
			src/MP3DConsole.cpp \
			src/MP3DMain.cpp \
			src/MP3DMainWindow.cpp 

HEADERS += \
			src/MP3DConfigureDlg.h \
			src/MP3DConsole.h \
			src/MP3DMainWindow.h 

			
