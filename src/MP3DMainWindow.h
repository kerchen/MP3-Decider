//
// Copyright 2011 Paul Kerchen
// Released under GNU GPL.  Enjoy.
//

#ifndef MP3DMAINWINDOW_H
#define MP3DMAINWINDOW_H

#include "ui_MainWindow.h"

#include <Phonon/AudioOutput>
#include <Phonon/SeekSlider>
#include <Phonon/MediaObject>
#include <Phonon/VolumeSlider>
#include <Phonon/BackendCapabilities>
#include <QDir>
#include <QFileInfo>

#include <map>

// Forward declarations
class QLabel;
class QShortcut;


namespace MP3DUI
{


/**@brief The main application window. */
class MainWindow : public QMainWindow
{
     Q_OBJECT

public:
	/// Standard constructor
	MainWindow(QWidget *parent = 0);
	/// Standard destructor
	virtual ~MainWindow();

	/**@brief Creates the context menu for the main application window.  
	 * Override of QMainWindow::createPopupMenu().
	 * Creates a popup context menu for the main application window.  The context
	 * menu includes items for toggling the visibility of the Toolbox window,
	 * the application toolbars and the console window.
	 */
	virtual QMenu * createPopupMenu ();

private slots:

	void onConfigureActionTriggered();
	
	/// Responds to the user requesting to exit the app.
	void onExitActionTriggered();

	/// Responds to requests to see the application's "about" info.
	//void onAboutActionTriggered();

	//void about();
	void stateChanged(Phonon::State newState, Phonon::State oldState);
	void tick(qint64 time);
	void sourceChanged(const Phonon::MediaSource &source);
	void metaStateChanged(Phonon::State newState, Phonon::State oldState);
	void aboutToFinish();	
	
	void playButtonClicked();
	void rewindButtonClicked();
	void fastForwardButtonClicked();
	void restartButtonClicked();
	void stopButtonClicked();
	
	void badTrackButtonClicked();
	void goodTrackButtonClicked();
	void skipTrackButtonClicked();
	
signals:

private:
	/**@brief Override of base function. */
	virtual void closeEvent( QCloseEvent* );


	bool copyFile( const QString& src, 
					const QString& destDir );

	bool loadNextSong();
	bool loadNextSong( const QString& rootDirName );
	bool loadSong( const QString& filename );
	bool loadNextSongSubdir(QDir& currentDirObject);
	
	void startNewSession();
	
	/// The Designer-generated user interface object.
	Ui::MainWindow		m_ui;


	//Phonon::SeekSlider*		m_seekSlider;
	Phonon::MediaObject*	m_mediaObject;
	Phonon::MediaObject*	m_metaInformationResolver;
	Phonon::AudioOutput*	m_audioOutput;
	Phonon::VolumeSlider*	m_volumeSlider;
	
	QString					m_metaInfoDisplay;
	int						m_metaInfoIndex;
	
	/// The starting directory for source tracks.
	QString					m_sourceDirString;
	
	/// The name of the directory that holds 'good' tracks.
	QString					m_goodDirString;
	
	/// The name of the directory that holds 'bad' tracks.
	QString					m_badDirString;
	
	/// The full path and filename of the track that is currently playing.
	QString					m_currentTrackFilename;
	//QDir					m_currentDirObject;
	//QFileInfoList			m_currentFileInfoList;
	bool					m_allFilesProcessed;
}; 

}	// namespace MP3DUI

#endif	// MP3DMAINWINDOW_H


