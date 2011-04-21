//
// Copyright 2010 Paul Kerchen
// Released under GNU GPL.  Enjoy.
//

#ifndef MP3DCONSOLE_H
#define MP3DCONSOLE_H

#include <QString>

#include <sstream>

class QPlainTextEdit;


namespace MP3D
{

/**@brief A singleton that defines the interface to the application error console.
*/
class Console
{
public:
	/**@brief Defines the severity levels for messages as well as the display threshold. 
	 * A message with severity NONE will never be displayed.  A display threshold of 
	 * NONE will prevent all messages from being displayed. */
	typedef enum
	{
		DEBUG,
		WARN,
		FATAL,
		NONE,
		ALWAYS	// Always prints message regardless of filtering.
	} Severity;

	/**@brief Console singleton accessor. 
	 * Note that this function also controls the timestamping of messages
	 * sent to the console.  Thus, if you write code that looks like this:
	 *	MP3D::Console::Instance() << "This is a mangled ";
	 *	MP3D::Console::Instance() << "sentence.\n";
	 * You will get this output:
	 *	12:43:19 (DEBUG): This is a mangled 12:43:19 (DEBUG): sentence.
	 * To avoid this, you can cache the pointer returned by this function:
	 *	MP3D::Console& con = Console::Instance();
	 *	con << "This is not a mangled ";
	 *	con << "sentence.\n";
	 * The above code would produce this output:
	 *	12:43:19 (DEBUG): This is not a mangled sentence.
     *	(Um, yes it is, dude...)
	 */
	static Console& Instance( Severity s = DEBUG )
	{
		if ( ! s_instance )
		{
			s_instance = new Console();
		}
		s_instance->m_printTimestamp = true;
		s_instance->m_nextMessageSeverity = s;
		return *s_instance;
	}

	/**@brief Tells the console what control it should send messages to. */
	void setMessageSink( QPlainTextEdit* sink );

	/**@brief Sets the minimum severity level a message must have to be displayed in the console.
	 */
	void setSeverityThreshold( Severity s ) { m_severityOutputThreshold = s; }

	/**@brief Returns the minimum severity level a message must have to be displayed in the console.
	 */
	Severity getSeverityThreshold() const { return m_severityOutputThreshold; }

	/**@brief Adds the given text to the console text.
	 * @param text The text to be sent to the console.
	 * If the message's severity is below the minimum severity display level,
	 * it will not be displayed.
	 */
	void appendText( const QString& );
	/**@brief Adds text to the console output.
	 * Behaves identically to appendText().
	 * @sa appendText
	 */
	Console& operator<<( const QString& );
	/**@brief Adds text to the console output.
	 * Behaves identically to appendText().
	 * @sa appendText
	 */
	template< typename T >
		Console& operator<<( const T& );

private:
	/// Private constructor.  Console::Instance() must be used.
	Console();
	/// Not implemented
	Console(const Console&);
	/// Not implemented
	void operator=(const Console&);

	/// The single instance of this class.
	static Console*	s_instance;
	/// The control that displays messages sent to the console.
	QPlainTextEdit*	m_sink;
	/// The severity that will be used for the next message sent to the console.
	Severity		m_nextMessageSeverity;
	/// The minimum severity level a message must have to be sent to the console.
	Severity		m_severityOutputThreshold;
	/// Controls printing of the timestamp so that it only appears at the start of a line.
	bool				m_printTimestamp;
	bool				m_lastTextWasHTML;
};

template< typename T >
Console& Console::operator<<( const T& msg )
{
	std::stringstream	ss;

	ss << msg;
	appendText(ss.str().c_str());
	return *this;
}

}	// namespace MP3D


#endif	// MP3DCONSOLE_H


