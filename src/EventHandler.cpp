#include "EventHandler.h"
#include "App2D.h"
#include "MenuSystem.h"

EventHandler::EventHandler( App2D &a ) : app(a) {

}

// Handle events, also the chat functionality
void EventHandler::Event( const sf::Event &evt ) {
	switch(evt.type) {
		case sf::Event::Closed:
			app.Close();
		break;

		case sf::Event::KeyPressed:
			if( evt.key.code == sf::Keyboard::Escape && app.inGame && !app.cinematicEngine.IsCinematicRunning() ) {
				app.inGame = false;
				MenuSystem *ms = new MenuSystem(app);
				app.AddEntity(ms, 1001, true);
				ms->DisplayMenu(Menu::PauseMenu);
			}

			keyPressEvent.notify(evt.key.code);
		break;

		case sf::Event::KeyReleased:
			keyReleaseEvent.notify(evt.key.code);
		break;

		case sf::Event::MouseButtonPressed:
			mousePressEvent.notify(evt.mouseButton.button);
		break;

		case sf::Event::MouseButtonReleased:
			mouseReleaseEvent.notify(evt.mouseButton.button);
		break;
	}
}


void EventHandler::RegisterGenericEventType( std::string type ) {
	genericEvents.push_back( std::make_pair( CppEvent1<bool, boost::any>(), type ) );
}

void EventHandler::NotifyGenericEvent( std::string type, boost::any data ) {
	for( int i = 0; i != genericEvents.size(); ++i ) {
		if(genericEvents[i].second == type) {
			genericEvents[i].first.notify(data);
			break;
		}
	}
}

CppEvent1<bool, boost::any> &EventHandler::GetGenericEvent( std::string type ) {
	for( int i = 0; i != genericEvents.size(); ++i ) {
		if(genericEvents[i].second == type) {
			return genericEvents[i].first;
		}
	}
	throw std::exception( "Nonexisting event requested" );
}

void EventHandler::Tick() {
	stepEvent.notify(app.GetFrameTime());
}
