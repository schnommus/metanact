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
		  currentMusic(0),
		  cinematicEngine(*this)
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

		GetEventHandler().RegisterGenericEventType("gravity");

		AddEntity( new SplashScreen(*this), 1001, true );

		sf::PostFX fx;
		fx.LoadFromFile("fisheye.sfx");
		fx.SetTexture("framebuffer", NULL);

		sf::Event evt;
		while( !isClosing ) {

			if( inGame && !cinematicEngine.IsCinematicRunning() ) {
				if( LevelChanged() ) LoadLevel(); // Might need to do level re-load
				CreateParticles();
				RespawnPlayerIfDead();
			}

			cinematicEngine.UpdateCinematic();

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

			PerformCameraMovement();

			// Draw the game field
			if( CountEntityOfType("warper") > 0 && !cinematicEngine.IsCinematicRunning() ) DrawGameField();

			// Call all normal draw functions, internally ordered by layer
			for( EntityMap::iterator it = entities.begin();
				 it != entities.end();
				 ++it) {
				if(it->first > 1000) { // Are we up to the UI?
					renderWindow.SetView(renderWindow.GetDefaultView());
				}
				// Only draw if it's relevent to a cinematic (assuming one's up)
				if (!cinematicEngine.IsCinematicRunning() || it->second->cinematicEntity)
					it->second->Draw();
			}

			renderWindow.SetView(renderWindow.GetDefaultView()); // make sure we are using a static view

			if(GetOption("MinimalUI") == "Disabled") DrawLogMessages();

			DrawSubtitles();

			// Draw minimap
			if(inGame && !cinematicEngine.IsCinematicRunning() ) DrawMinimap();

			// If we're about to change levels, show loading text before the game freezes
			if( LevelChanged() && inGame && !cinematicEngine.IsCinematicRunning() ) {
				sf::String n( sf::Unicode::Text("Loading..."), FindFont("Action_Force.ttf", 60), 60 );
				n.SetPosition( 0, 0 );
				n.SetColor( sf::Color(255, 255, 255, 200) );
				Draw(n);
			}

			renderWindow.SetView(gameView); // Back to the normal view
			
			int esize=50;
			for( EntityMap::iterator it = entities.begin();
				it != entities.end();
				++it) {
					if( it->second->type == "warper" && renderWindow.ConvertCoords( 0, 0 ).x-esize < it->second->x &&
						renderWindow.ConvertCoords( 0, 0 ).y-esize < it->second->y &&
						renderWindow.ConvertCoords( GetSize().x, GetSize().y ).x+esize > it->second->x &&
						renderWindow.ConvertCoords( GetSize().x, GetSize().y ).y+esize > it->second->y) {
						sf::Vector2f bLeft = renderWindow.ConvertCoords(0, 0);
						sf::Vector2f tRight = renderWindow.ConvertCoords(GetSize().x, GetSize().y);
						fx.SetParameter("mouse", (it->second->x - bLeft.x)/GetSize().x, 1.0-(it->second->y - bLeft.y)/GetSize().y);
						Draw(fx);
					}
			}

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

long long App2D::AddEntity( Entity *e , int layer, bool isViewEntity) {
	e->id = isViewEntity?nextViewId():nextId();
	entities.insert( std::make_pair(layer, e ) );
	return e->id;
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

void App2D::DrawSubtitles() 
{
	if(currentDialogue != "") { // Draw subtitles
		sf::String n( sf::Unicode::Text(currentSubtitles), FindFont("Temp7c.ttf", 15), 15 );
		n.SetPosition( GetSize().x/2 - n.GetCharacterPos(currentSubtitles.size()-1).x/2, GetSize().y-100 );
		n.SetColor( sf::Color(255, 255, 255, 255) );
		sf::FloatRect backgroundRect = n.GetRect();
		sf::Shape r = sf::Shape::Rectangle(backgroundRect.Left, backgroundRect.Bottom, backgroundRect.Right, backgroundRect.Top, sf::Color::Black);
		Draw(r);
		Draw(n);
	}

	// Dialogue has to be checked for 'stoppage' every frame
	for( std::vector<sf::Sound*>::iterator it = playingSounds.begin(); it != playingSounds.end(); ++it ) {
		if( *it == currentDialoguePtr && (*it)->GetStatus() == sf::Sound::Stopped) {
			currentDialogue = "";
			currentDialoguePtr = nullptr;
			delete (*it);
			playingSounds.erase(it);
			break;
		}
	}
}

void App2D::DisplayBigMessage(std::string message) {
	AddEntity( new BigMessage(*this, message), 1001, true );
}

void App2D::CreateParticles() 
{
	// Make binary particles for effect
	if(binaryReplaceTimer.GetElapsedTime() > 0.05f  * EvaluateOption("ParticleDensity"))	 {
		AddEntity( new BinaryParticle(*this), 1 );
		binaryReplaceTimer.Reset();
	}
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



sf::Sound *App2D::PlaySound( std::string sound, bool loop, bool useDist, int x, int y, int vol ) {
	sound = "../media/sound/" + sound;
	if( soundMap.find(sound) == soundMap.end() ) {
		if(!soundMap[sound].LoadFromFile(sound)) {
			throw std::exception(std::string(sound + " could not be loaded").c_str());
		}
	}
	if( !useDist ) { // Use distance from camera as volume
		playingSounds.push_back( new sf::Sound(soundMap[sound], loop, 1, vol) );
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
				if( *it == currentDialoguePtr ) // Keep track of when dialogue is finished for subtitles
					currentDialogue = "";
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
	static std::string lastMusic;
	if( lastMusic != music ) {
		if( currentMusic ) currentMusic->Stop();
		currentMusic = PlaySound(music, true);
	}
	lastMusic = music;
}

void App2D::PlayDialogue(std::string filename) {
	// First, rid of any previous dialogue
	for( std::vector<sf::Sound*>::iterator it = playingSounds.begin(); it != playingSounds.end(); ++it ) {
		if( *it == currentDialoguePtr ) {
			delete (*it);
			playingSounds.erase(it);
			break;
		}
	}

	currentDialoguePtr = PlaySound(filename, false, false, 0, 0, 30);
	currentDialogue = filename;

	// Fetch subtitles
	std::ifstream ifs;
	ifs.open("../media/sound/!subtitles.stt", std::ios::in);
	if( ifs.is_open() ) {
		while( !ifs.eof() ) {
			std::string all; getline(ifs, all);
			std::string n, msg;
			if ( all.find('=') != std::string::npos) // If there's an '='
				n = all.substr(0, all.find('=')), msg = all.substr(all.find('=')+1, all.size()-1 );
			if( n == filename) { currentSubtitles = msg; break; }
		}
		ifs.close();
	} else {
		std::cout << "Dialogue loaded without subtitles: " << filename << std::endl;
		currentSubtitles = "DIALOGUE";
	}
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
		if( !it->second->type.empty() && it->second->isEnemy ) {
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
			if( it->second->displayName == ".." ) s.SetColor( sf::Color( 255, 255, 0 ) );
			s.SetPosition( renderWindow.GetWidth() - sx + (abs(field.bleft.x)+it->second->x)*fx,
						   renderWindow.GetHeight() - sy + (abs(field.bleft.y)+it->second->y)*fy );
			Draw(s);
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
		if( GetOption(type) == "MELT!" ) return 0.1;
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

void App2D::ReleaseFile( std::string subname, std::string message ) {
	std::string writepath;
	for( int i = 0; i != currentPath.size(); ++i ) {
		if( currentPath[i] == '/' || currentPath[i] == '\\' || currentPath[i] == ':' )
			writepath.append(1, '_');
		else
			writepath.append(1, currentPath[i]);
	}
	std::ofstream ofs;
	ofs.open(std::string("saves/"+writepath+".sav"), std::ios::out | std::ios::app);
	ofs << subname << "=" << message << std::endl;
	ofs.close();
}

std::string App2D::CheckFile( std::string subname ) {
	std::string writepath;
	for( int i = 0; i != currentPath.size(); ++i ) {
		if( currentPath[i] == '/' || currentPath[i] == '\\' || currentPath[i] == ':' )
			writepath.append(1, '_');
		else
			writepath.append(1, currentPath[i]);
	}
	std::ifstream ifs;
	ifs.open(std::string("saves/"+writepath+".sav"), std::ios::in);
	if( !ifs.is_open() ) {
		return ""; // Nothing has been logged in this directory yet
	}
	
	while( !ifs.eof() ) {
		std::string all; getline(ifs, all);
		std::string n, msg;
		if ( all.find('=') != std::string::npos) // If there's an '='
			n = all.substr(0, all.find('=')), msg = all.substr(all.find('=')+1, all.size()-1 );

		if( n == subname ) {
			ifs.close();
			return msg;
		}
	}

	ifs.close();
	return "";
}

void App2D::WipeCurrentGame() {
	// Goodbye to the player's current game..
	inGame = false;
	for ( App2D::EntityMap::iterator it = entities.begin(); it != entities.end(); ++it ) {
		if( it->second->type != "" )
			RemoveEntity(it->second->id);
	}
	currentPath = ""; // Wipe so deletion thinks we're changing levels
	oldpath = " ";
	ExecuteDeletionQueue();
	currentPath = GetOption("InitialDirectory");
}

void App2D::ReEnterGame() {
	std::ifstream ifs;
	ifs.open("saves/!header.sav", std::ios::in);
	if( ifs.is_open() ) {
		while( !ifs.eof() ) {
			std::string all; getline(ifs, all);
			std::string n, msg;
			if ( all.find('=') != std::string::npos) // If there's an '='
				n = all.substr(0, all.find('=')), msg = all.substr(all.find('=')+1, all.size()-1 );
			if( n == "currentPath") currentPath = msg;
			std::cout << "Got current path from file: " << currentPath << std::endl;
		}
		ifs.close();
	} else {
		// Get initial directory
		currentPath = GetOption("InitialDirectory");
	}
	inGame = true;
	SetMusic("ambience.ogg");
}

bool App2D::LevelChanged() {
	return oldpath != currentPath;
}

using namespace boost::filesystem;

struct SizeCmp {
	bool operator() (path p1, path p2 ) {
		return file_size(p1) > file_size(p2);
	}
};

void App2D::LoadLevel() {
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
		if( CheckFile(files[i].filename().string()) != "destroyed" ) {
			std::ifstream ifs;
			ifs.open("../media/entity/!enemytypes.edb", std::ios::in);

			if( !ifs.is_open() ) {
				throw std::exception("Couldn't open enemy types database for reading");
			}

			// Read type to spawn from file. Note that the entities MUST be in descending order
			while( !ifs.eof() ) {
				long long size; std::string etype;
				ifs >> size >> etype;
				if( file_size(files[i]) > size ) {
					AddEntity( new DefinedEntity( *this, etype, rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, files[i].filename().string() ), 10 );
					break;
				}
			}

			ifs.close();
		}
	}


	AddEntity( new DefinedEntity( *this, "spawner" ), 10 );

	std::ofstream ofs;
	ofs.open("saves/!header.sav", std::ios::out | std::ios::trunc);

	if( !ofs.is_open() ) {
		throw std::exception("Couldn't open !header file for writing");
	} else {
		ofs << "currentPath=" << currentPath;
	}

	int r = rand()%8;
	if( r == 0 ) PlayDialogue("lchange1.ogg");
	if( r == 1 ) PlayDialogue("lchange2.ogg");
	if( r == 2 ) PlayDialogue("lchange3.ogg");
	if( r == 3 ) PlayDialogue("lchange4.ogg");
}

void App2D::RespawnPlayerIfDead() {
	// Make sure there is a player
	if( playerDeathTimer.GetElapsedTime() > 3.0f && CountEntityOfType("localplayer") == 0) {
		playerName = GetOption("PlayerName");
		AddEntity( new DefinedEntity( *this, "localplayer", 150, 150, sf::Vector2f(), 0, true, playerName ), 10 );
	}
}

void App2D::PerformCameraMovement() {
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
}

void App2D::DrawGameField() {
	Draw( sf::Shape::Line(field.bleft.x, field.bleft.y, field.tright.x, field.bleft.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );
	Draw( sf::Shape::Line(field.bleft.x, field.bleft.y, field.bleft.x, field.tright.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );
	Draw( sf::Shape::Line(field.bleft.x, field.tright.y, field.tright.x, field.tright.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );
	Draw( sf::Shape::Line(field.tright.x, field.tright.y, field.tright.x, field.bleft.y, 3, sf::Color( 0, 0, 0 ), 1, sf::Color(50, 50, 255) ) );
}

void App2D::DrawLogMessages() {
	// Draw messages, scrolling them up when new ones arrive (if minimal UI is disabled)
	for( int i = messageList.size()-maxMessages, j = 0; i != messageList.size(); ++i, ++j ) {
		sf::String n( sf::Unicode::Text(messageList[i]), FindFont("BlackWolf.ttf", 12), 12 );
		n.SetPosition( 0, renderWindow.GetHeight()-maxMessages*15+j*15-5 );
		n.SetColor( sf::Color(255, 255, 255, 200) );
		Draw(n);
	}
}

// Does a previous savefile exist?
bool App2D::IsLastSave() {
	std::ifstream ifs;
	ifs.open("saves/!header.sav", std::ios::in);
	if( ifs.is_open() ) {
		ifs.close();
		return true;
	}
	return false;
}