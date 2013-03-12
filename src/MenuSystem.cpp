#include "MenuSystem.h"
#include <fstream>
#include <boost/filesystem.hpp>

// BEGIN MENU DEFINITIONS (Add new screens to enum in 'MenuSystem.h')

// DEFINITIONS FOR "MAIN MENU"

class ContinueGame : public MenuItem {
public:
	ContinueGame( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Continue Exploration";

		if( app.IsLastSave() ) {
			disabled = false;
		} else {
			disabled = true;
		}
	}

	virtual void Clicked() {
		if(!disabled) {
			ms.EnterGame();
		}
	}
};

class StartGame : public MenuItem {
public:
	StartGame( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "New Exploration";
	}
	virtual void Clicked() {
		if( !app.IsLastSave() ) { // If there's no existing savefile
			ms.EnterGame(); // run game
		} else {
			// Otherwise make the player confirm
			ms.DisplayMenu(Menu::NewGameMenu);
		}
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
		items.push_back( boost::shared_ptr<MenuItem>( new ContinueGame(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new StartGame(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new Options(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new Exit(app, ms) ) );
	}
};

// DEFINITIONS FOR "PAUSE MENU"

class ResumeGame : public MenuItem {
public:
	ResumeGame( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Resume Game";
	}
	virtual void Clicked() {
		ms.EnterGame();
	}
};

class ToMainMenu : public MenuItem {
public:
	ToMainMenu( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Back to Main Menu";
	}
	virtual void Clicked() {
		app.WipeCurrentGame();
		app.SetMusic("metmain.ogg");
		ms.DisplayMenu(Menu::MainMenu);
	}
};

class PauseMenu : public MenuScreen {
public:
	PauseMenu( App2D &a, MenuSystem &m ) : MenuScreen(a, m) {
		title = "Paused";
		items.push_back( boost::shared_ptr<MenuItem>( new ResumeGame(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new Options(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new ToMainMenu(app, ms) ) );
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

class ParticleDensity : public MenuItem {
public:
	ParticleDensity( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Particle Density";
		attributes.push_back("Heavy");
		attributes.push_back("Sparse");
		attributes.push_back("MELT!");

		InitializeToCurrent(attributes, currentAttribute, "ParticleDensity", app);
	}
	virtual void Clicked() {
		CycleOptions(attributes, currentAttribute, "ParticleDensity", app);
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
		app.DisplayBigMessage("Won't take effect until the game is restarted!");
	}
};

class MinimalUI : public MenuItem {
public:
	MinimalUI( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Minimal UI";
		attributes.push_back("Enabled");
		attributes.push_back("Disabled");

		InitializeToCurrent(attributes, currentAttribute, "MinimalUI", app);
	}
	virtual void Clicked() {
		CycleOptions(attributes, currentAttribute, "MinimalUI", app);
	}
};

class GoBack : public MenuItem {
public:
	GoBack( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Back";
	}
	virtual void Clicked() {
		// Be careful when going back from the options screen!
		// Could be either the main menu or the pause menu.
		if( app.CountEntityOfType("warper") > 0 )
			ms.DisplayMenu(Menu::PauseMenu);
		else
			ms.DisplayMenu(Menu::MainMenu);
	}
};

class OptionsMenu : public MenuScreen {
public:
	OptionsMenu( App2D &a, MenuSystem &m ) : MenuScreen(a, m) {
		title = "Options";
		items.push_back( boost::shared_ptr<MenuItem>( new PlayerName(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new ControlScheme(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new ParticleDensity(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new MinimalUI(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new Directory(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new GoBack(app, ms) ) );
	}
};

// DEFINITIONS FOR "NEW GAME MENU"

class YesNewGame : public MenuItem {
public:
	YesNewGame( App2D &a, MenuSystem &m ) : MenuItem(a, m) {
		name = "Yep, erase previous save";
	}
	virtual void Clicked() {
		// Erase all previous .sav files here
		using namespace boost::filesystem;
		path cwd( current_path() / "saves" );
		std::cout << cwd << " GDDF" << std::endl;
		directory_iterator end;
		for( directory_iterator di(cwd); di != end; ++di ) {
			if( di->path().extension() == ".sav") {
				std::cout << di->path() << " deleted" << std::endl;
				remove(di->path());
			}
		}

		// Then start a new game
		ms.EnterGame();

	}
};

class NewGameMenu : public MenuScreen {
public:
	NewGameMenu( App2D &a, MenuSystem &m ) : MenuScreen(a, m) {
		title = "Are you sure?";
		items.push_back( boost::shared_ptr<MenuItem>( new YesNewGame(app, ms) ) );
		items.push_back( boost::shared_ptr<MenuItem>( new GoBack(app, ms) ) );
	}
};

// END MENU DEFINITIONS

MenuSystem::MenuSystem( App2D &a ) : Entity(a) {
	// Register events
	hStepEvent = app.GetEventHandler().stepEvent.attach(this, &MenuSystem::onStep);

	menus.push_back( boost::shared_ptr<MenuScreen>( new MainMenu(app, *this) ) );
	menus.push_back( boost::shared_ptr<MenuScreen>( new OptionsMenu(app, *this) ) );
	menus.push_back( boost::shared_ptr<MenuScreen>( new PauseMenu(app, *this) ) );
	menus.push_back( boost::shared_ptr<MenuScreen>( new NewGameMenu(app, *this) ) );

	DisplayMenu(Menu::MainMenu);

	mText.SetColor( sf::Color(255, 255, 255, 255) );
	mText.SetFont(app.FindFont("Action_Force.ttf", 40));
}

bool MenuSystem::onStep(float delta) {
	// Particle effect and camera shifting

	app.CreateParticles();

	// Only move camera if not in pause screen
	if( app.CountEntityOfType("warper")  == 0 ) app.cdelta.x = 10;

	return true;
}

void MenuSystem::Draw() {
	currentMenu = newCurrentMenu;

	mTextOSS.str(""); //Clear OSS
	mTextOSS << currentMenu->title;
	mText.SetText( mTextOSS.str() );
	mText.SetSize(40);

	mText.SetPosition(0, 0);
	mText.SetPosition(app.GetSize().x/2 - mText.GetCharacterPos(mTextOSS.str().size()-1).x/2, app.GetSize().y/2-currentMenu->items.size()*25-40 );

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
		mText.SetPosition(app.GetSize().x/2 - mText.GetCharacterPos(mTextOSS.str().size()-1).x/2, i*50 + 50 + app.GetSize().y/2 - currentMenu->items.size()*25 );

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

		if( currentMenu->items[i]->hovering && !currentMenu->items[i]->disabled )
			mText.SetColor( sf::Color(255, 255, 255, 255) );
		else if ( !currentMenu->items[i]->disabled )
			mText.SetColor( sf::Color(255, 255, 255, 190) );
		else
			mText.SetColor( sf::Color(255, 255, 255, 100) );

		app.Draw(mText);
	}
};