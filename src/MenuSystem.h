#ifndef MENUSYSTEM_H
#define MENUSYSTEM_H

#include "App2D.h"
#include <vector>
#include <sstream>
#include <boost/smart_ptr.hpp>

class MenuSystem;

class MenuItem {
public:
	MenuItem( App2D &a, MenuSystem &m ) : app(a), ms(m), currentAttribute(0), hovering(false), disabled(false), downed(false) { }
	virtual void Clicked() { }
	App2D &app;
	MenuSystem &ms;
	std::string name;
	bool hovering;
	bool downed;
	bool disabled;
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

// TO ADD A NEW MENU SCREEN, add here AND in constructor of MenuSystem
namespace Menu {
	enum Type {
		MainMenu = 0,
		OptionsMenu = 1,
		PauseMenu = 2,
		NewGameMenu = 3
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
		app.ReEnterGame();
		app.RemoveEntity(this->id);
	}

	void DisplayMenu(Menu::Type index) {
		newCurrentMenu = menus[index].get();
	}

private:
	CppEventHandler hStepEvent;
	sf::Clock binaryReplaceTimer;
	sf::String mText;
	std::ostringstream mTextOSS;
	std::vector<boost::shared_ptr<MenuScreen>> menus;
	MenuScreen *currentMenu, *newCurrentMenu;
};

#endif