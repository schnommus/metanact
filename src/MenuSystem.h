#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

#include "App2D.h"
#include <vector>
#include <sstream>
#include <boost/smart_ptr.hpp>

class MenuSystem;

class MenuItem {
public:
	MenuItem( App2D &a, MenuSystem &m ) : app(a), ms(m), currentAttribute(0), hovering(false) { }
	virtual void Clicked() { }
	App2D &app;
	MenuSystem &ms;
	std::string name;
	bool hovering;
	bool downed;
	int currentAttribute;
	std::vector<std::string> attributes;
};

class MenuScreen {
public:
	MenuScreen( App2D &a, MenuSystem &m) : app(a), ms(m) { }
	App2D &app;
	MenuSystem &ms;
	std::string title;
	std::vector<boost::shared_ptr<MenuItem>> items;
};

namespace Menu {
	enum Type {
		MainMenu = 0,
		OptionsMenu = 1
	};
}

class MenuSystem : public Entity {
public:
	MenuSystem( App2D &a );

	~MenuSystem() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta);

	virtual void Draw();

	void EnterGame() {
		app.inGame = true;
		app.SetMusic("ambience.ogg");
		app.RemoveEntity(this->id);
	}

	void DisplayMenu(Menu::Type index) {
		newCurrentMenu = menus[index].get();
	}

private:
	CppEventHandler hStepEvent;
	sf::Clock binaryReplaceTimer;
	sf::Sprite arrow;
	sf::String mText;
	std::ostringstream mTextOSS;
	std::vector<boost::shared_ptr<MenuScreen>> menus;
	MenuScreen *currentMenu, *newCurrentMenu;
};

#endif