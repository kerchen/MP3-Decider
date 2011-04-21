//
// Copyright 2011 Paul Kerchen
// Released under GNU GPL.  Enjoy.
//

#include "MP3DMainWindow.h"
//#include "MP3DApp.h"
#include "MP3DConsole.h"

#include "ui_MainWindow.h"

#include <QMessageBox>


#include <iostream>
#include <fstream>

//#include <windows.h>
//#include <Wincon.h>


void qtMessageHandler(QtMsgType type, const char *msg)
{
	MP3D::Console&	con = MP3D::Console::Instance();

	switch (type) {
	case QtDebugMsg:
		con << "QtDebug: ";
		break;
	case QtWarningMsg:
		con << "QtWarning: ";
		break;
	case QtCriticalMsg:
		con << "QtCritical: ";
		break;
	case QtFatalMsg:
		con << "QtFatal: ";
		break;
	}
	con << msg << "\n";
}


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MP3DUI::MainWindow mainWindow;

	qInstallMsgHandler(qtMessageHandler);

	//AllocConsole(); 
	//freopen("conin$","r",stdin); 
	//freopen("conout$","w",stdout); 
	//freopen("conout$","w",stderr);

	mainWindow.show();

	return app.exec();
} 


