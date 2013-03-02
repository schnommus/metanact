#include "MenuSystem.h"
#include "DefinedEntity.h"

// BEGIN MENU DEFINITIONS (Add new screens to enum in 'MenuSystem.h')

// DEFINITIONS FOR "MAIN MENU"

class StartGame : public MenuItem {
public:
	StartGame( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "New Exploration";
		// Any warpers? Must be in the 'Escape' menu.
		if( app.CountEntityOfType("warper") > 0 ) {
			name = "Resume";
		}
	}
	virtual void Clicked() {
		ms.EnterGame();
	}
};

class Options : public MenuItem {
public:
	Options( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Game Options";
	}
	virtual void Clicked() {
		ms.DisplayMenu(Menu::OptionsMenu);
	}
};

class Exit : public MenuItem {
public:
	Exit( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Exit Game";
	}
	virtual void Clicked() {
		app.Close();
	}
};

class MainMenu : public MenuScreen {
public:
	MainMenu( App2D &a, MenuSystem &m ) : MenuScreen(a, m) {
		title = "Main Menu";
		items.push_back( boost::shared_ptr<MenuItem>( new StartGame(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new Options(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new Exit(app, ms) ) );
	}
};

// DEFINITIONS FOR "OPTIONS"


void AllowNewDefinitions(std::vector<std::string> &attributes, std::string optionName, App2D &app ) {
	if( std::find( attributes.begin(), attributes.end(), app.GetOption(optionName) ) == attributes.end() ) {
		attributes.push_back(app.GetOption(optionName));
	}
}

void InitializeToCurrent(std::vector<std::string> &attributes, int &currentAttribute, std::string optionName, App2D &app ) {
	currentAttribute = std::distance( attributes.begin(), std::find(attributes.begin(), attributes.end(), app.GetOption(optionName) ) );
	currentAttribute = (currentAttribute > attributes.size()-1 ? 0 : currentAttribute);
}

void CycleOptions(std::vector<std::string> &attributes, int &currentAttribute, std::string optionName, App2D &app ) {
	if( ++currentAttribute == attributes.size() ) currentAttribute = 0;
	app.SetOption(optionName, attributes[currentAttribute]);
}

class ControlScheme : public MenuItem {
public:
	ControlScheme( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Control Scheme";
		attributes.push_back("Global Movement");
		attributes.push_back("Mouse Relative");

		InitializeToCurrent(attributes, currentAttribute, "ControlScheme", app);
	}
	virtual void Clicked() {
		CycleOptions(attributes, currentAttribute, "ControlScheme", app);
	}
};

class PlayerName : public MenuItem {
public:
	PlayerName( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Player Name";
		attributes.push_back("Anonymous");
		attributes.push_back("Sausage");
		attributes.push_back("Getron");
		attributes.push_back("Fyerk");
		attributes.push_back("Dinkle");
		attributes.push_back("Dangle");
		attributes.push_back("Schnommus");

		AllowNewDefinitions(attributes, "PlayerName", app);

		InitializeToCurrent(attributes, currentAttribute, "PlayerName", app);
	}
	virtual void Clicked() {
		CycleOptions(attributes, currentAttribute, "PlayerName", app);
	}
};

class Directory : public MenuItem {
public:
	Directory( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Initial Directory";
		attributes.push_back("C:\\");
		attributes.push_back("C:\\Windows\\");

		AllowNewDefinitions(attributes, "InitialDirectory", app);

		InitializeToCurrent(attributes, currentAttribute, "InitialDirectory", app);
	}
	virtual void Clicked() {
		CycleOptions(attributes, currentAttribute, "InitialDirectory", app);
		app.DisplayMessage("Changes here won't take effect until the game is restarted!");
	}
};

class GoBack : public MenuItem {
public:
	GoBack( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Back";
	}
	virtual void Clicked() {
		ms.DisplayMenu(Menu::MainMenu);
	}
};

class OptionsMenu : public MenuScreen {
public:
	OptionsMenu( App2D &a, MenuSystem &m ) : MenuScreen(a, m) {
		title = "Options";
		items.push_back( boost::shared_ptr<MenuItem>( new ControlScheme(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new PlayerName(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new Directory(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new GoBack(app, ms) ) );
	}
};

// END MENU DEFINITIONS

MenuSystem::MenuSystem( App2D &a ) : Entity(a) {
	// Register events
	hStepEvent = app.GetEventHandler().stepEvent.attach(this, &MenuSystem::onStep);

	menus.push_back( boost::shared_ptr<MenuScreen>( new MainMenu(app, *this) ) );
	menus.push_back( boost::shared_ptr<MenuScreen>( new OptionsMenu(app, *this) ) );

	DisplayMenu(Menu::MainMenu);

	arrow.SetImage(app.FindImage("arrowSolid.png"));
	arrow.SetPosition(100, 100);

	mText.SetColor( sf::Color(255, 255, 255, 255) );
	mText.SetFont(app.FindFont("Action_Force.ttf", 40));
}

bool MenuSystem::onStep(float delta) {

	// Particle effect and camera shifting
	if(binaryReplaceTimer.GetElapsedTime() > 0.05f)	 {
		app.AddEntity( new BinaryParticle(app), 1 );
		binaryReplaceTimer.Reset();
	}
	app.cdelta.x = 10;

	return true;
}

void MenuSystem::Draw() {
	currentMenu = newCurrentMenu;

	//app.Draw(arrow);

	mTextOSS.str(""); //Clear OSS
	mTextOSS << currentMenu->title;
	mText.SetText( mTextOSS.str() );
	mText.SetSize(40);

	mText.SetPosition(0, 0);
	mText.SetPosition(app.GetSize().x/2 - mText.GetCharacterPos(mTextOSS.str().size()-1).x/2, app.GetSize().y/2-currentMenu->items.size()*60 );

	mText.SetColor( sf::Color(255, 255, 255, 255) );
	app.Draw(mText);

	for(int i = 0; i != currentMenu->items.size(); ++i ) {
		mTextOSS.str(""); //Clear OSS
		mTextOSS << currentMenu->items[i]->name;

		// Do attribute processing
		if(!currentMenu->items[i]->attributes.empty()) {
			mTextOSS << " [" << currentMenu->items[i]->attributes[currentMenu->items[i]->currentAttribute] << "]";
		}

		mText.SetText( mTextOSS.str() );
		mText.SetSize(25);

		mText.SetPosition(0, 0);
		mText.SetPosition(app.GetSize().x/2 - mText.GetCharacterPos(mTextOSS.str().size()-1).x/2, i*50 + 50 + app.GetSize().y/2 - currentMenu->items.size()*50 );

		//Check whether item is hovered over, pressed, or clicked
		if( mText.GetPosition().x < app.GetInput().GetMouseX() &&
			mText.GetPosition().x+mText.GetCharacterPos(mTextOSS.str().size()-1).x > app.GetInput().GetMouseX() &&
			mText.GetPosition().y < app.GetInput().GetMouseY() &&
			mText.GetPosition().y+mText.GetSize() > app.GetInput().GetMouseY() ) {
			currentMenu->items[i]->hovering = true;
			if(app.GetInput().IsMouseButtonDown(sf::Mouse::Left))
				currentMenu->items[i]->downed = true;
			if(!app.GetInput().IsMouseButtonDown(sf::Mouse::Left) && currentMenu->items[i]->downed) {
				currentMenu->items[i]->Clicked();
				currentMenu->items[i]->downed = false;
			}
		} else {
			currentMenu->items[i]->hovering = false;
			currentMenu->items[i]->downed = false;
		}

		if( currentMenu->items[i]->hovering )
			mText.SetColor( sf::Color(255, 255, 255, 255) );
		else
			mText.SetColor( sf::Color(255, 255, 255, 190) );

		app.Draw(mText);
	}
};