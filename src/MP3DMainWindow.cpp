//
// Copyright 2011 Paul Kerchen
// Released under GNU GPL.  Enjoy.
//




#include <QUiLoader>
#include <QMessageBox>
#include <QLabel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>
#include <QTime>

#include "MP3DMainWindow.h"
//#include "MP3DApp.h"
#include "MP3DConsole.h"
#include "MP3DConfigureDlg.h"

#include <assert.h>

static const QString	VERSION_STR( "0.1.0" );

using namespace MP3D;

namespace MP3DUI
{


MainWindow::MainWindow(QWidget *parent)
: QMainWindow( parent )
, m_allFilesProcessed( false )
{
	m_ui.setupUi(this);

	QFont	f( "courier" );
	m_ui.m_logTextEdit->setFont(f);
	
	Console::Instance().setMessageSink( m_ui.m_logTextEdit );
	// Change WARN to DEBUG to get debugging messages in log window.
	Console::Instance().setSeverityThreshold( Console::WARN );	
	
	// Actions
//	connect(m_ui.actionExit, SIGNAL(triggered()), SLOT(onExitActionTriggered()));

	Console::Instance( Console::ALWAYS ) << tr("Greets from The MP3 Decider, version %1.\n").arg(VERSION_STR);
	
	m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	m_mediaObject = new Phonon::MediaObject(this);
	m_metaInformationResolver = new Phonon::MediaObject(this);

	m_mediaObject->setTickInterval(1000);
	connect(m_mediaObject, 
		SIGNAL(tick(qint64)), 
		SLOT(tick(qint64)));
	connect(m_mediaObject, 
		SIGNAL(stateChanged(Phonon::State, Phonon::State)),
		SLOT(stateChanged(Phonon::State, Phonon::State)));
	connect(m_metaInformationResolver, 
		SIGNAL(stateChanged(Phonon::State,Phonon::State)),
		SLOT(metaStateChanged(Phonon::State, Phonon::State)));
	connect(m_mediaObject, 
		SIGNAL(currentSourceChanged(const Phonon::MediaSource &)),
		SLOT(sourceChanged(const Phonon::MediaSource &)));
	connect(m_mediaObject, 
		SIGNAL(aboutToFinish()), 
		SLOT(aboutToFinish()));

	connect(m_ui.actionConfigure,
		SIGNAL(triggered()),
		SLOT(onConfigureActionTriggered()));		
	connect(m_ui.actionExit,
		SIGNAL(triggered()),
		SLOT(onExitActionTriggered()));
		
	connect( m_ui.m_playToolButton,
		SIGNAL(clicked(bool)),
		SLOT(playButtonClicked()));
	connect( m_ui.m_rewindToolButton,
		SIGNAL(clicked(bool)),
		SLOT(rewindButtonClicked()));
	connect( m_ui.m_fastForwardToolButton,
		SIGNAL(clicked(bool)),
		SLOT(fastForwardButtonClicked()));
	connect( m_ui.m_restartToolButton,
		SIGNAL(clicked(bool)),
		SLOT(restartButtonClicked()));
	connect( m_ui.m_stopToolButton,
		SIGNAL(clicked(bool)),
		SLOT(stopButtonClicked()));
	connect( m_ui.m_badTrackToolButton,
		SIGNAL(clicked(bool)),
		SLOT(badTrackButtonClicked()));
	connect( m_ui.m_skipTrackToolButton,
		SIGNAL(clicked(bool)),
		SLOT(skipTrackButtonClicked()));
	connect( m_ui.m_goodTrackToolButton,
		SIGNAL(clicked(bool)),
		SLOT(goodTrackButtonClicked()));
		
	Phonon::createPath(m_mediaObject, m_audioOutput);
	
	m_mediaObject->stop();
	m_mediaObject->clearQueue();

	startNewSession();
}


MainWindow::~MainWindow()
{
}

QMenu * MainWindow::createPopupMenu()
{
	QMenu* myMenu = new QMenu();
#if 0
	myMenu->addAction(m_ui.actionToggleToolboxWindow);
	myMenu->addAction(m_ui.m_consoleDockWidget->toggleViewAction());
	myMenu->addAction(m_ui.m_statusDockWidget->toggleViewAction());
	myMenu->addAction(m_ui.m_activationDockWidget->toggleViewAction());
	myMenu->addAction(m_ui.m_watchDockWidget->toggleViewAction());
	myMenu->addAction(m_ui.actionToggleExperimentToolbar);
	myMenu->addAction(m_ui.actionToggleScenarioToolbar);
	myMenu->addAction(m_ui.actionToggleWindowToolbar);
#endif
	return(myMenu);
}


void MainWindow::onConfigureActionTriggered()
{
	ConfigureDlg	confDlg(this);
	QString			oldSource( m_sourceDirString );
	QString			oldBadDir( m_badDirString );
	QString			oldGoodDir( m_goodDirString );
	QString			oldCurrentTrack( m_currentTrackFilename );
	
	confDlg.setStartingDirName( m_sourceDirString );
	if ( ! m_goodDirString.isEmpty() )
		confDlg.setGoodSubdirName( m_goodDirString );
	if ( ! m_badDirString.isEmpty() )
		confDlg.setBadSubdirName( m_badDirString );
	
	tryAgain:
	if ( QDialog::Accepted != confDlg.exec() )
	{
		m_sourceDirString = oldSource;
		m_badDirString = oldBadDir;
		m_goodDirString = oldGoodDir;
		m_currentTrackFilename = oldCurrentTrack;
		return;
	}
	
	m_sourceDirString = confDlg.getStartingDirName();
	m_badDirString = confDlg.getBadSubdirName();
	m_goodDirString = confDlg.getGoodSubdirName();
	m_currentTrackFilename = "";

	if ( ! loadNextSong() )
	{
		QString	msg( tr("There are no unprocessed files in %1.\nSelect another source directory." ).arg(m_sourceDirString) );
		QMessageBox::warning( this, "Nothing to do!", msg );
		goto tryAgain;
	}
	
}


void MainWindow::onExitActionTriggered()
{
	close();
}


void MainWindow::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
	switch (newState) 
	{
	 case Phonon::ErrorState:
		 if (m_mediaObject->errorType() == Phonon::FatalError) {
			 QMessageBox::warning(this, tr("Fatal Error"),
			 m_mediaObject->errorString());
		 } else {
			 QMessageBox::warning(this, tr("Error"),
			 m_mediaObject->errorString());
		 }
		 break;
	 case Phonon::PlayingState:
			 //playAction->setEnabled(false);
			 //pauseAction->setEnabled(true);
			 //stopAction->setEnabled(true);
			 break;
	 case Phonon::StoppedState:
			 //stopAction->setEnabled(false);
			 //playAction->setEnabled(true);
			 //pauseAction->setEnabled(false);
			 //timeLcd->display("00:00");
			 break;
	 case Phonon::PausedState:
			 //pauseAction->setEnabled(false);
			 //stopAction->setEnabled(true);
			 //playAction->setEnabled(true);
			 break;
	 case Phonon::BufferingState:
			 break;
	 default:
		 ;
	}
}

void MainWindow::tick(qint64 time)
{
	QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

	m_ui.m_trackTime->display(displayTime.toString("mm:ss"));
}



void MainWindow::sourceChanged(const Phonon::MediaSource &)
{
	m_ui.m_trackTime->display("00:00");
}

void MainWindow::metaStateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
	if (newState == Phonon::ErrorState) 
	{
		QString	msg( m_metaInformationResolver->errorString() );
		
		Console::Instance(Console::WARN) << tr("Error opening files: ") << msg << "\n";
		QMessageBox::warning(this, tr("Error opening files"), msg );
		return;
	}

	if (newState != Phonon::StoppedState && newState != Phonon::PausedState)
		return;

	if (m_metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid)
		return;

	QMap<QString, QString> metaData = m_metaInformationResolver->metaData();

	QString title( metaData.value("TITLE") );
	if (title == "")
		title = m_metaInformationResolver->currentSource().fileName();

	m_ui.m_trackTitleLabel->setText( title );
	m_ui.m_trackArtistLabel->setText( metaData.value("ARTIST") );
	m_ui.m_trackAlbumLabel->setText( metaData.value("ALBUM") );
	
	m_mediaObject->setCurrentSource(m_metaInformationResolver->currentSource());

	//Phonon::MediaSource source = m_metaInformationResolver->currentSource();
	//m_metaInformationResolver->setCurrentSource(sources.at(index));

	m_mediaObject->play();
	m_ui.m_playToolButton->setIcon( QIcon( ":/MainWindow/PauseButton.png" ) );
}


void MainWindow::aboutToFinish()
{
	//mediaObject->enqueue(sources.at(index));
	//m_ui.m_playToolButton->setIcon( QIcon( ":/MainWindow/PlayButton.png" ) );
	//m_mediaObject->stop();
}


void MainWindow::playButtonClicked()
{
	if ( m_mediaObject->state() == Phonon::StoppedState ||
	     m_mediaObject->state() == Phonon::PausedState )
	{
		m_mediaObject->play();
		m_ui.m_playToolButton->setIcon( QIcon( ":/MainWindow/PauseButton.png" ) );
	}
	else if ( m_mediaObject->state() == Phonon::PlayingState )
	{
		m_mediaObject->pause();
		m_ui.m_playToolButton->setIcon( QIcon( ":/MainWindow/PlayButton.png" ) );
	}
}


void MainWindow::rewindButtonClicked()
{
	if ( m_mediaObject->currentTime() > 5000 )
		m_mediaObject->seek( m_mediaObject->currentTime() - 5000 );
}


void MainWindow::fastForwardButtonClicked()
{
	if ( m_mediaObject->remainingTime() > 5000 )
		m_mediaObject->seek( m_mediaObject->currentTime() + 5000 );
}



void MainWindow::restartButtonClicked()
{
	m_mediaObject->seek( 0 );
}


void MainWindow::stopButtonClicked()
{
	m_mediaObject->stop();
	m_mediaObject->seek(0);
	m_ui.m_playToolButton->setIcon( QIcon( ":/MainWindow/PlayButton.png" ) );
	m_ui.m_trackTime->display("00:00");
}



void MainWindow::badTrackButtonClicked()
{	
	if ( m_allFilesProcessed )
		return;
		
	Console::Instance(Console::ALWAYS) << tr("Marking '%1' as BAD.\n").arg(m_currentTrackFilename);
	QString	lastTrack( m_currentTrackFilename );
	bool removeFile( copyFile( m_currentTrackFilename, m_badDirString ) );
	loadNextSong();
	if ( removeFile && ! QFile::remove( lastTrack ) )
		Console::Instance(Console::WARN) << tr("Could not delete original source file '%1'.\n").arg(lastTrack);


	if ( m_allFilesProcessed )
	{
		QString	msg( tr("All files in %1 have been classified.\nSelect another source directory." ).arg(m_sourceDirString) );
		QMessageBox::warning( this, tr("All done!"), msg );
		startNewSession();
		return;
	}
}


void MainWindow::goodTrackButtonClicked()
{
	if ( m_allFilesProcessed )
		return;
		
	Console::Instance(Console::ALWAYS) << tr("Marking '%1' as GOOD.\n").arg(m_currentTrackFilename);
	QString	lastTrack( m_currentTrackFilename );
	bool removeFile( copyFile( m_currentTrackFilename, m_goodDirString ) );
	loadNextSong();
	if ( removeFile && ! QFile::remove( lastTrack ) )
		Console::Instance(Console::WARN) << tr("Could not delete original source file '%1'.\n").arg(lastTrack);

	if ( m_allFilesProcessed )
	{
		QString	msg( tr("All files in %1 have been classified.\nSelect another source directory." ).arg(m_sourceDirString) );
		QMessageBox::warning( this, tr("All done!"), msg );
		startNewSession();
		return;
	}
}


void MainWindow::skipTrackButtonClicked()
{
	if ( m_allFilesProcessed )
		return;
		
	Console::Instance(Console::ALWAYS) << tr("Deferring a decision on '%1'.\n").arg(m_currentTrackFilename);
	loadNextSong();
	if ( m_allFilesProcessed )
	{
		QString	msg( tr("All files in %1 have been classified.\nSelect another source directory." ).arg(m_sourceDirString) );
		QMessageBox::warning( this, tr("All done!"), msg );
		startNewSession();
		return;
	}
}



void MainWindow::closeEvent( QCloseEvent* event )
{
	// Copy the log to a file in the starting directory.
	QString	logFilename( m_sourceDirString + "/MP3DeciderLog_" );
	
	logFilename += QDate::currentDate().toString( "yyyy_MM_dd" );
	logFilename += QTime::currentTime().toString( "_HHmm" );
	logFilename += ".txt";
	
	QFile	logFile( logFilename );
	
	if ( ! logFile.open( QFile::WriteOnly ) )
	{
		if ( QMessageBox::No == QMessageBox::warning( this, tr("File error"), 
									tr("Couldn't write log data to '%1'.\nDo you still wish to exit?").arg( logFilename ), 
									QMessageBox::Yes | QMessageBox::No ) )
		{
			event->ignore();
			return;
		}
	}
	
	QTextStream out(&logFile);
    out << m_ui.m_logTextEdit->toPlainText();
	logFile.close();
	event->accept();
}



bool MainWindow::copyFile( const QString& src, 
							const QString& destSubdirString )
{
	QFileInfo	fi( src );
	QString		destDirString( fi.absolutePath() + "/" + destSubdirString );
	QString		destFile( destDirString + "/" + fi.fileName() );
	QDir		srcDir( fi.absolutePath() );
	
	if ( ! srcDir.exists( destSubdirString ) )
	{
		if ( ! srcDir.mkdir( destSubdirString ) )
		{
			Console::Instance(Console::FATAL) << tr("Cannot create directory '%1'.\n").arg(destDirString);
		}
	}
	Console::Instance(Console::ALWAYS) << tr("Copying '%1' to '%2'.\n").arg(src).arg(destFile);
	
	if ( ! QFile::copy( src, destFile ) )
	{
		if ( QFile::exists(destFile) )
			Console::Instance(Console::FATAL) << tr("Cannot overwrite existing file '%1'.\n").arg(destFile);
		else
			Console::Instance(Console::FATAL) << tr("Could not copy '%1' to '%2'.\n").arg(src).arg(destFile);
		return false;
	}
	
	return true;
}

bool MainWindow::loadNextSong()
{
	m_mediaObject->stop();
	m_mediaObject->clearQueue();

	if ( ! m_allFilesProcessed && loadNextSong( m_sourceDirString ) )
	{
		//m_mediaObject->play();
		//m_ui.m_playToolButton->setIcon( QIcon( ":/MainWindow/PauseButton.png" ) );
		return true;
	}
	return false;
}



bool MainWindow::loadNextSong( const QString& rootDirString )
{
	QString	currentDirString( rootDirString );
	QString	lastSongFilename;
	
	if ( ! m_currentTrackFilename.isEmpty() )
	{
		QFileInfo	fi( m_currentTrackFilename );
		
		currentDirString = fi.absolutePath();
		lastSongFilename = fi.fileName();
	}
	
	
	QStringList	filters;

	filters << "*.mp3";
	
	if ( lastSongFilename.isEmpty() )
	{
		// Hain't loaded nothin' yet.  Grab the first mp3 we find and go with it.
		QDir currentDirObject = QDir( currentDirString );
		QFileInfoList currentFileInfoList = currentDirObject.entryInfoList( filters, QDir::Files );
		
		if ( ! currentFileInfoList.isEmpty() )
		{
			int	fileNo = 0, limit = currentFileInfoList.count();
			
			while ( fileNo < limit && ! loadSong( currentFileInfoList[fileNo].absoluteFilePath() )  )
				++fileNo;
			
			if ( fileNo < limit )
				return true;
		}
		
		// No (valid) mp3's in current directory.  Dive deeper.
		return loadNextSongSubdir(currentDirObject);
	}
	else
	{
		int	lastIndex = -1;

		// Find the current track in the directory.
		QDir currentDirObject = QDir( currentDirString );
		QFileInfoList currentFileInfoList = currentDirObject.entryInfoList( filters, QDir::Files );
		
		for ( int i = 0, limit = currentFileInfoList.count(); i < limit; ++i )
		{
			QString	fname( currentFileInfoList[i].fileName() );

			if ( currentFileInfoList[i].fileName() == lastSongFilename )
			{
				lastIndex = i;
				break;
			}
		}

		// If we played a track from this directory, it should still be there at this point.
		assert( lastIndex != -1 );
		
		// Play the first mp3 we find after the current track.
		for ( int i = lastIndex + 1, limit = currentFileInfoList.count(); i < limit; ++i )
		{
			if ( currentFileInfoList[i].isFile() )
				return loadSong( currentFileInfoList[i].absoluteFilePath() );
		}
		
		// No more mp3's in the current directory.  See if there is something deeper.
		if ( ! loadNextSongSubdir(currentDirObject) )
		{
			// Nothing deeper.  Go up.
			QStringList	filters;
			
			if ( currentDirObject == QDir(m_sourceDirString) )
			{
				Console::Instance(Console::ALWAYS) << tr("Processed all audio files in '%1'\n").arg(m_sourceDirString);
				m_allFilesProcessed = true;
				return false;
			}

			while( true )
			{
				QString		currentSubDir( currentDirObject.dirName() );
				
				Console::Instance() << tr("Going up a directory from '%1'\n").arg(currentDirObject.absolutePath());
				if ( ! currentDirObject.cdUp() )
				{
					Console::Instance(Console::WARN) << tr("Couldn't change directory up from '%1'\n").arg(currentDirObject.absolutePath());
					return false;
				}

				// Get the index of the sub dir we were just in.
				currentFileInfoList = currentDirObject.entryInfoList( filters, QDir::AllDirs | QDir::NoDotAndDotDot );

				lastIndex = -1;
				for ( int i = 0, limit = currentFileInfoList.count(); i < limit; ++i )
				{
					if ( currentFileInfoList[i].fileName() == currentSubDir )
					{
						lastIndex = i;
						break;
					}
				}
				
				assert( lastIndex != -1 );
				
				// See if this was the last sub dir of the current directory.
				if ( lastIndex == currentFileInfoList.count() - 1 )
				{
					if ( currentDirObject == QDir(m_sourceDirString) )
					{
						Console::Instance(Console::ALWAYS) << tr("Processed all audio files in '%1'\n").arg(m_sourceDirString);
						m_allFilesProcessed = true;
						return false;
					}
					// Pop up to the next level.
					continue;
				}
				
				// Dive into the next sub dir.  Reset current track filename so that directory navigation
				// works properly.
				m_currentTrackFilename = "";
				for ( int i = lastIndex + 1, limit = currentFileInfoList.count(); i < limit; ++i )
				{
					if ( loadNextSong( currentFileInfoList[i].absoluteFilePath() ) )
						return true;
				}
			}
		}
		return true;
	}
}


bool MainWindow::loadNextSongSubdir(QDir& currentDirObject)
{
	QStringList	filters;
	
	QFileInfoList currentFileInfoList = currentDirObject.entryInfoList( filters, QDir::AllDirs | QDir::NoDotAndDotDot );
	if ( currentFileInfoList.isEmpty() )
	{
		Console::Instance() << "Reached dead-end in directory tree.\n";
		return false;
	}
	
	for ( int i = 0, limit = currentFileInfoList.count(); i < limit; ++i )
	{
		if ( currentFileInfoList[i].fileName() == m_goodDirString ||
			 currentFileInfoList[i].fileName() == m_badDirString )
		{
			Console::Instance() << "Skipping good/bad dir.\n";
			continue;
		}
		
		Console::Instance() << "Diving down into '" << currentFileInfoList[i].absoluteFilePath() << "'\n";
		if ( loadNextSong( currentFileInfoList[i].absoluteFilePath() ) )
		{
			return true;
		}
	}
	Console::Instance( Console::DEBUG ) << "Reached dead-end in directory tree.\n";
	return false;			
}

bool MainWindow::loadSong( const QString& filename )
{
	Phonon::MediaSource source(filename);

	m_metaInformationResolver->setCurrentSource(source);
	m_mediaObject->setCurrentSource(source);
	m_ui.m_trackFilenameLabel->setText( filename );

	Console::Instance(Console::ALWAYS) << "Loading \"" << filename << "\"\n";
	m_currentTrackFilename = filename;
	
	return true;
}

void MainWindow::startNewSession()
{
	ConfigureDlg	confDlg(this);

	confDlg.setStartingDirName( m_sourceDirString );
	if ( ! m_goodDirString.isEmpty() )
		confDlg.setGoodSubdirName( m_goodDirString );
	if ( ! m_badDirString.isEmpty() )
		confDlg.setBadSubdirName( m_badDirString );
	
	// If they don't set a configuration, bail out.
	tryAgain:
	if ( QDialog::Accepted != confDlg.exec() )
	{
		close();
		return;
	}

	m_sourceDirString = confDlg.getStartingDirName();
	m_badDirString = confDlg.getBadSubdirName();
	m_goodDirString = confDlg.getGoodSubdirName();
	m_currentTrackFilename = "";
	m_allFilesProcessed = false;
	
	if ( ! loadNextSong() )
	{
		QString	msg( QString("There are no unprocessed files in %1.\nSelect another source directory." ).arg(m_sourceDirString) );
		QMessageBox::warning( this, "Nothing to do!", msg );
		goto tryAgain;
	}
}



}; // Namespace MP3DUI


