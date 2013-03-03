#include "App2D.h"
#include "DefinedEntity.h"

#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <boost\shared_ptr.hpp>
#include <boost\filesystem.hpp>


App2D::App2D( bool useVSync,
			  bool useCurrentResolution,
			  const std::string &title,
			  bool fullscreen,
			  std::string pname,
			  int size_x,
			  int size_y)

		: renderWindow( ( useCurrentResolution ? sf::VideoMode::GetDesktopMode() : sf::VideoMode( size_x, size_y )),
		   title,
		   ( ( fullscreen ) ? sf::Style::Fullscreen : sf::Style::Close ), sf::WindowSettings( 24, 8, 4 ) ), // 4xAA
		  eventHandler(*this),
		  vSync(useVSync),
		  isClosing(false),
		  following(false),
		  playerName(pname),
		  maxMessages(5), // Number of messages on the screen at once
		  inGame(false),
		  currentId(1),
		  currentViewId(1),
		  currentMusic(0)
	{

	// Maybe make this alterable
	renderWindow.EnableKeyRepeat(true);

	renderWindow.UseVerticalSync(useVSync);

	gameView = renderWindow.GetDefaultView();

	gameView.Move(-GetSize().x/2, -GetSize().y/2); // Start a little closer to the centre

	// Initialise the message scroller
	for( int i = 0; i != maxMessages; ++i ) {
		messageList.push_back("");
	}

}

App2D::~App2D() {
	// Entities would be destroyed automatically - but then there is
	// a chance that event objects will die before the entities'
	// handles have a chance to detach from them.
	entities.clear();
}

void App2D::Run() {
	try {
		DisplayMessage("Metanact v0.1a - Engine Initialized."); // Moved some stuff here from the constructor so exceptions can be caught

		SetMusic("metmain.ogg");

		LoadOptions();

		currentPath = GetOption("InitialDirectory");

		GetEventHandler().RegisterGenericEventType("gravity");

		AddEntity( new SplashScreen(*this), 1001, true );

		sf::Event evt;
		while( !isClosing ) {

			if( inGame ) {

				if (CountEntityOfType("grunt") == 0) currentLevelUnlocked = true;
				LoadLevel(); // Might need to do level re-load

				// Make binary particles for effect
				if(binaryReplaceTimer.GetElapsedTime() > 0.05f  * EvaluateOption("ParticleDensity"))	 {
					AddEntity( new BinaryParticle(*this), 1 );
					binaryReplaceTimer.Reset();
				}
			}

			// Make sure there is a player
			if( inGame && playerDeathTimer.GetElapsedTime() > 3.0f && CountEntityOfType("localplayer") == 0) {
				playerName = GetOption("PlayerName");
				AddEntity( new DefinedEntity( *this, "localplayer", 150, 150, sf::Vector2f(), 0, true, playerName ), 10 );
			}

			while( renderWindow.GetEvent(evt) ) {
				eventHandler.Event(evt);
			}
			eventHandler.Tick();

			
			// If the framerate is too high and we are using VSync, limit it.
			// This can happen if VSync doesn't work properly.
			if( vSync && (1.0f/GetFrameTime()) > 100 ) {
				renderWindow.SetFramerateLimit(60);
			} else {
				// Probably shouldn't do this every frame - but I haven't measured any consequences.
				renderWindow.SetFramerateLimit(0);
			}

			renderWindow.Clear();
			
			renderWindow.SetView(gameView);


			//Camera controls
			if( following ) {
				cdelta.x = toFollow->x-gameView.GetCenter().x;
				cdelta.y = toFollow->y-gameView.GetCenter().y;
			}
			// Camera slowdown
			cdelta.x += (-cdelta.x*GetFrameTime());
			cdelta.y += (-cdelta.y*GetFrameTime());
			float cSpd = 5; // Speed of camera following
			gameView.Move(cdelta.x*GetFrameTime()*cSpd, cdelta.y*GetFrameTime()*cSpd);


			// Draw the game field
			Draw( sf::Shape::Line(field.bleft.x, field.bleft.y, field.tright.x, field.bleft.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );
			Draw( sf::Shape::Line(field.bleft.x, field.bleft.y, field.bleft.x, field.tright.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );
			Draw( sf::Shape::Line(field.bleft.x, field.tright.y, field.tright.x, field.tright.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );
			Draw( sf::Shape::Line(field.tright.x, field.tright.y, field.tright.x, field.bleft.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );

			// Call all normal draw functions, internally ordered by layer
			for( EntityMap::iterator it = entities.begin();
				 it != entities.end();
				 ++it) {
				if(it->first > 1000) { // Are we up to the UI?
					renderWindow.SetView(renderWindow.GetDefaultView());
				}
				it->second->Draw();
			}

			renderWindow.SetView(renderWindow.GetDefaultView()); // make sure we are using a static view

			// Draw messages, scrolling them up when new ones arrive
			for( int i = messageList.size()-maxMessages, j = 0; i != messageList.size(); ++i, ++j ) {
				sf::String n( sf::Unicode::Text(messageList[i]), FindFont("BlackWolf.ttf"), 15 );
				n.SetPosition( 0, renderWindow.GetHeight()-maxMessages*18+j*18-5 );
				n.SetColor( sf::Color(255, 255, 255, 200) );
				Draw(n);
			}

			// Draw minimap
			if(inGame) DrawMinimap();

			// If we're about to change levels, show loading text before the game freezes
			if( oldpath != currentPath && inGame ) {
				sf::String n( sf::Unicode::Text("Loading..."), FindFont("Action_Force.ttf", 60), 60 );
				n.SetPosition( 0, 0 );
				n.SetColor( sf::Color(255, 255, 255, 200) );
				Draw(n);
			}

			renderWindow.SetView(gameView); // Back to the normal view

			renderWindow.Display();

			// Destroy entities added for removal
			ExecuteDeletionQueue();
		}
		renderWindow.Close();
	} catch ( const std::exception &e ) {
		std::cout << "Unhandled exception: " << e.what() << std::endl;
	} catch ( ... ) {
		std::cout << "Non-standard unhandled exception!" << std::endl;
	}
	renderWindow.Close();
	system("pause");
}

void App2D::Draw(const sf::Drawable &object) {
	renderWindow.Draw( object );
}

const sf::Input &App2D::GetInput() {
	return renderWindow.GetInput();
}

void App2D::Close() {
	isClosing = true;
}

sf::Vector2i App2D::GetSize() const {
	return sf::Vector2i(renderWindow.GetWidth(), renderWindow.GetHeight());
}

EventHandler &App2D::GetEventHandler() {
	return eventHandler;
}

float App2D::GetFrameTime() const {
	return renderWindow.GetFrameTime();
}

long long App2D::nextId() {
	return ++currentId;
}

long long App2D::nextViewId() {
	return 1e6 + ++currentViewId;
}

int App2D::GetObjectCount() {
	return entities.size();
}

void App2D::AddEntity( Entity *e , int layer, bool isViewEntity) {
	e->id = isViewEntity?nextViewId():nextId();
	entities.insert( std::make_pair(layer, e ) );
}

Entity *App2D::GetEntityWithId( long long idToGet ) {
	for( EntityMap::iterator it = entities.begin();
		 it != entities.end();
		 ++it) {
			if( idToGet == it->second->id ) {
				return it->second.get();
			}
	}
	return 0;
}

void App2D::RemoveEntity( long long id ) {
	idRemovalList.push_back(id);
}

void App2D::FollowEntity( Entity &e ) {
	toFollow = &e;
	following = true;
}

void App2D::StopFollowing() {
	following = false;
}

void App2D::ExecuteDeletionQueue() {
	// Destroy entities added for removal
	for ( std::vector<int>::iterator i = idRemovalList.begin(); i != idRemovalList.end(); ++i ) {
		for ( EntityMap::iterator it = entities.begin(); it != entities.end(); ++it ) {
			if( it->second->id == *i) {
				entities.erase( it );
				break;
			}
		}
	}
	idRemovalList.clear();
}

void App2D::DisplayMessage(std::string message) {
	if(!message.empty()) {
		messageList.push_back(message);
	}
}

void App2D::DisplayBigMessage(std::string message) {
	AddEntity( new BigMessage(*this, message), 1001, true );
}

// Some manager functions to avoid resource duplicates

sf::Font &App2D::FindFont( std::string dir, int size ) {
	dir = "../media/font/" + dir;
	if( fontMap.find(dir) == fontMap.end() ) {
		std::cout << "New font: " << dir << std::endl;
		if(!fontMap[dir].LoadFromFile(dir, size)) {
			throw std::exception(std::string(dir + " could not be loaded").c_str());
		}
	}
	return fontMap.find(dir)->second;
}

sf::Image &App2D::FindImage( std::string dir ) {
	dir = "../media/image/" + dir;
	if( imageMap.find(dir) == imageMap.end() ) {
		std::cout << "New image: " << dir << std::endl;
		if(!imageMap[dir].LoadFromFile(dir)) {
			throw std::exception(std::string(dir + " could not be loaded").c_str());
		}
	}
	return imageMap.find(dir)->second;
}

sf::Sound *App2D::PlaySound( std::string sound, bool loop, bool useDist, int x, int y ) {
	sound = "../media/sound/" + sound;
	if( soundMap.find(sound) == soundMap.end() ) {
		if(!soundMap[sound].LoadFromFile(sound)) {
			throw std::exception(std::string(sound + " could not be loaded").c_str());
		}
	}
	if( !useDist ) { // Use distance from camera as volume
		playingSounds.push_back( new sf::Sound(soundMap[sound], loop) );
	} else {
		float dist = sqrt(powf(x-gameView.GetCenter().x, 2) + powf(y-gameView.GetCenter().y, 2));
		float v = 100-(dist/20.0);
		playingSounds.push_back( new sf::Sound(soundMap[sound], loop, 1, v > 0 ? v : 0 ) );
	}
	playingSounds.back()->Play();

	// Get rid of all the sounds that have stopped playing
	bool isErasing = true; // Boolean required 'cause of iterator invalidation
	while(isErasing) {
		isErasing = false;
		for( std::vector<sf::Sound*>::iterator it = playingSounds.begin(); it != playingSounds.end(); ++it ) {
			if( (*it)->GetStatus() == sf::Sound::Stopped) {
				delete (*it);
				playingSounds.erase(it);
				isErasing = true;
				break;
			}
		}
	}

	return playingSounds.back();
}

void App2D::SetMusic(std::string music) {
	if( currentMusic ) currentMusic->Stop();
	currentMusic = PlaySound(music, true);
}

bool App2D::InField( Entity *e ) {
	if( e->x < field.bleft.x ) { return false; }
	if( e->y < field.bleft.y ) { return false; }
	if( e->x > field.tright.x ) { return false; }
	if( e->y > field.tright.y ) { return false; }
	return true;
}

void App2D::DrawMinimap() {
	double sx = 180, sy = 140;
	double fx = sx/abs(field.bleft.x - field.tright.x);
	double fy = sy/abs(field.bleft.y - field.tright.y);

	sf::Shape box = sf::Shape::Rectangle( renderWindow.GetWidth()-sx, renderWindow.GetHeight()-sy,
										  renderWindow.GetWidth()-1, renderWindow.GetHeight()-1, sf::Color(0, 0, 0, 150), 1, sf::Color(180, 180, 180) );
	Draw(box);
	sf::Shape s = sf::Shape::Line(0, 0, 1, 1, 3, sf::Color(255, 255, 255) );

	for ( EntityMap::iterator it = entities.begin(); it != entities.end(); ++it ) {
		if( !it->second->type.empty() && it->second->type == "grunt" ) {
			s.SetColor( sf::Color( 255, 0, 0 ) );
			s.SetPosition( renderWindow.GetWidth() - sx + (abs(field.bleft.x)+it->second->x)*fx,
						   renderWindow.GetHeight() - sy + (abs(field.bleft.y)+it->second->y)*fy );
			Draw(s);
		} else if ( !it->second->type.empty() && it->second->type == "localplayer" ) {
			s.SetColor( sf::Color( 0, 255, 0 ) );
			s.SetPosition( renderWindow.GetWidth() - sx + (abs(field.bleft.x)+it->second->x)*fx,
						   renderWindow.GetHeight() - sy + (abs(field.bleft.y)+it->second->y)*fy );
			Draw(s);
		} else if ( !it->second->type.empty() && it->second->type == "warper") {
			s.SetColor( sf::Color( 0, 0, 255 ) );
			s.SetPosition( renderWindow.GetWidth() - sx + (abs(field.bleft.x)+it->second->x)*fx,
						   renderWindow.GetHeight() - sy + (abs(field.bleft.y)+it->second->y)*fy );
			if(currentLevelUnlocked) Draw(s);
		} else if( !it->second->type.empty() && InField(it->second.get()) ) {
			s.SetColor( sf::Color( 128, 128, 128 ) );
			s.SetPosition( renderWindow.GetWidth() - sx + (abs(field.bleft.x)+it->second->x)*fx,
						   renderWindow.GetHeight() - sy + (abs(field.bleft.y)+it->second->y)*fy );
			Draw(s);
		}
	}
}

int App2D::CountEntityOfType( std::string type ) {
	int count = 0;
	for ( EntityMap::iterator it = entities.begin(); it != entities.end(); ++it ) {
		if( it->second->type == type )
			++count;
	}
	return count;
}

std::string App2D::GetOption( std::string type ) {
	try {
		return gameOptions[type];
	} catch( ... ) {
		std::cout << "Invalid option accessed! - " << type << std::endl;
		return "";
	}
}

float App2D::EvaluateOption( std::string type) {
	if(type == "ParticleDensity") {
		if( GetOption(type) == "Heavy" ) return 1.0;
		if( GetOption(type) == "Sparse" ) return 3.0;
	}

	throw std::exception("Attempted to evaluate option without numerical type!");
}

void App2D::SetOption( std::string type, std::string data ) {
	gameOptions[type] = data;
	SaveOptions();
}

void App2D::SaveOptions() {
	std::ofstream ofs;
	ofs.open("options.cfg", std::ios::out | std::ios::trunc);

	if( !ofs.is_open() ) {
		throw std::exception("Couldn't open options file for writing!");
	}

	std::map< std::string, std::string >::iterator it;
	for( it = gameOptions.begin(); it != gameOptions.end(); ++it ) {
		ofs << it->first << "=" << it->second << std::endl;
	}

	ofs.close();
}

void App2D::LoadOptions() {
	std::ifstream ifs;
	ifs.open("options.cfg", std::ios::in);

	if( !ifs.is_open() ) {
		throw std::exception("Couldn't open options file for reading!");
	}

	while( !ifs.eof() ) {
		std::string all; getline(ifs, all);
		if ( all.find('=') != std::string::npos) // If there's an '='
			SetOption( all.substr(0, all.find('=')), all.substr(all.find('=')+1, all.size()-1 ) );
	}

	ifs.close();
}

using namespace boost::filesystem;

struct SizeCmp {
	bool operator() (path p1, path p2 ) {
		return file_size(p1) > file_size(p2);
	}
};

void App2D::LoadLevel() {
	if( oldpath == currentPath ) return;

	currentLevelUnlocked = false;

	// Delete all existing entities (except player)
	for ( EntityMap::iterator it = entities.begin(); it != entities.end(); ++it ) {
		if( it->second->type != "localplayer" )
			RemoveEntity(it->second->id);
		else {
			it->second->inWarp = true;
			it->second->vel = sf::Vector2f();
			it->second->warpId = 0; // Warp 0 is spawner

			// Resetting all ids here
			currentId = it->second->id + 1;
		}
	}

	currentViewId = 1;

	ExecuteDeletionQueue();

	AddEntity(new FPSMeter(*this), 1001, true);

	// Parse a directory getting names of all files + folders
	path p(currentPath);
	currentPath = canonical(p).string();
	oldpath = currentPath;

	directory_iterator end;
	std::vector<path> dirs;
	std::vector<path> files;
	for( directory_iterator di(p); di != end; ++di ) {
		if( is_regular_file(di->path()) ) {
			files.push_back(di->path());
		} else if ( is_directory(di->path()) ) {
			dirs.push_back(di->path());
		} else {
			std::cout << di->path() << " is an invalid path" << std::endl;
		}
	}

	std::random_shuffle( files.begin(), files.end() );
	std::random_shuffle( dirs.begin(), dirs.end() );

	// The maximum amounts of files etc.
	if( files.size() > 60 )
		files.resize(60);
	if( dirs.size() > 60 )
		dirs.resize(60);


	int gameSize = 500 + 50*files.size() + 150*dirs.size();
	std::ostringstream s;
	s << gameSize/1000 + 1;
	DisplayMessage(std::string("Entered ") + currentPath + " -- Approximate size: " + s.str() + " ByteParsec(s)");

	field.bleft.x = -gameSize/2;
	field.bleft.y = -gameSize/2;
	field.tright.x = gameSize/2;
	field.tright.y = gameSize/2;

	for( int i = 0; i != dirs.size(); ++i ) {
		//std::cout << "Directory: " << dirs[i].filename() << std::endl;
		bool crashed = false;
		path p = dirs[i];
		directory_iterator end;
		int countFiles=0, countDirs = 0;
		try {
			for( directory_iterator di(p); di != end; ++di ) {
				if( is_regular_file(di->path()) ) {
					++countFiles; if(countFiles > 60) countFiles = 60;
				} else if ( is_directory(di->path()) ) {
					++countDirs; if(countDirs > 60) countDirs = 60;
				} else {
					std::cout << di->path() << " is an invalid path (while doing subsearch)" << std::endl;
				}
			}
		} catch (...) {
			std::cout << "Crash while doing subsearch in " << dirs[i].string() << std::endl;
			crashed = true;
		}
		if( !crashed ) {
			float scale = (0.7+float(countFiles+countDirs)/90);
			AddEntity( new DefinedEntity( *this, "warper", rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, dirs[i].filename().string(), scale ), 10 );
		} else {
			DisplayMessage("\'" + dirs[i].filename().string() + "\' is an innaccessible subdirectory - not spawning wormhole.");
		}
	}
	// Add a warper that allows the player to go up a directory
	AddEntity( new DefinedEntity( *this, "warper", rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, ".." ), 10 );

	std::sort( files.begin(), files.end(), SizeCmp() );

	for( int i = 0; i != files.size(); ++i ) {
		//std::cout << "File: " << files[i].filename() << " size: " << file_size(files[i]) << std::endl;
		if( file_size(files[i]) > 1e4 ) // Greater than a 1k
			AddEntity( new DefinedEntity( *this, "grunt", rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, files[i].filename().string() ), 10 );
		else
			AddEntity( new DefinedEntity( *this, "scrap", rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, files[i].filename().string() ), 10 );
	}


	AddEntity( new DefinedEntity( *this, "spawner" ), 10 );
}