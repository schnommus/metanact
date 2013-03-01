#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <utility>
#include "CppEvent.h"
#include "boost\any.hpp"

class App2D;

/// Used for Entities to attach to events
class EventHandler {
public:
	EventHandler( App2D& );
	void Event( const sf::Event& );
	void Tick();

	/// Triggered every frame. Has one float parameter, the frame delta.
	CppEvent1<bool, float> stepEvent;

	/// Triggered on a key press. The parameter is an sf::Key code.
	CppEvent1<bool, sf::Key::Code> keyPressEvent;

	/// Triggered on a key release. The parameter is an sf::Key code.
	CppEvent1<bool, sf::Key::Code> keyReleaseEvent;

	/// Triggered on a mouse button press. The parameter is an sf::Mouse button.
	CppEvent1<bool, sf::Mouse::Button> mousePressEvent;

	/// Triggered on a mouse button release. The parameter is an sf::Mouse button.
	CppEvent1<bool, sf::Mouse::Button> mouseReleaseEvent;

	/// Custom events that new types can be made to suit
	std::vector< std::pair< CppEvent1<bool, boost::any>, std::string > > genericEvents;

	void RegisterGenericEventType( std::string type );

	void NotifyGenericEvent( std::string type, boost::any data );

	CppEvent1<bool, boost::any> &GetGenericEvent( std::string type );

private:
	EventHandler( const EventHandler& ); // Noncopyable
	EventHandler &operator=( const EventHandler& ); 
	App2D &app;
};

#endif