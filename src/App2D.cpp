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

		: renderWindow( ( useCurrentResolution ? sf::VideoMode::getDesktopMode() : sf::VideoMode( size_x, size_y )),
		   title,
		   ( ( fullscreen ) ? sf::Style::Fullscreen : sf::Style::Close ), sf::ContextSettings( 24, 8, 4 ) ), // 4xAA
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
		  cinematicEngine(*this),
		  playerData(*this)
	{

	pastDeltaClock = 0;

	// Maybe make this alterable
	renderWindow.setKeyRepeatEnabled(true);

	renderWindow.setVerticalSyncEnabled(useVSync);

	gameView = renderWindow.getDefaultView();

	gameView.move(-GetSize().x/2, -GetSize().y/2); // Start a little closer to the centre

	persistanceTarget.create(GetSize().x, GetSize().y);
	persistanceTarget.clear(sf::Color(0, 0, 0, 0));

	currentPlayerScore = 0;

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

		playerData.Init();

		GetEventHandler().RegisterGenericEventType("gravity");

		AddEntity( new SplashScreen(*this), 1001, true );

		sf::Event evt;
		while( !isClosing ) {

			deltaClock.restart();

			if( inGame && !cinematicEngine.IsCinematicRunning() ) {
				if( LevelChanged() ) LoadLevel(); // Might need to do level re-load
				CreateParticles();
				RespawnPlayerIfDead();
			}

			cinematicEngine.UpdateCinematic();

			while( renderWindow.pollEvent(evt) ) {
				eventHandler.Event(evt);
			}
			eventHandler.Tick();

			renderWindow.clear();
			
			renderWindow.setView(gameView);
			
			sf::RectangleShape rs(sf::Vector2f( GetSize().x, GetSize().y) );
			rs.setPosition(gameView.getCenter().x-GetSize().x/2, gameView.getCenter().y-GetSize().y/2);
			rs.setFillColor(sf::Color(0, 0, 0, 3000*GetFrameTime()));
			persistanceTarget.draw(rs);
			persistanceTarget.setView(gameView);
			persistanceTarget.display();

			//Draw the persistent stuff
			sf::Sprite sp(persistanceTarget.getTexture());
			sp.setPosition(gameView.getCenter().x-GetSize().x/2, gameView.getCenter().y-GetSize().y/2);
			//Draw(sp);

			sf::Shader *pc = &FindShader("persistanceclean.sfx");
			sf::RenderStates res(pc);
			renderWindow.draw(sp, res);


			// Draw the game field
			if( CountEntityOfType("warper") > 0 && !cinematicEngine.IsCinematicRunning() ) DrawGameField();

			bool upToUI = false;

			// Call all normal draw functions, internally ordered by layer
			for( EntityMap::iterator it = entities.begin();
				 it != entities.end();
				 ++it) {
				if(it->first > 1000 && !upToUI) { // Are we up to the UI?
					upToUI = true;
					DrawPostProcessingEffects();
					if( !inGame && !cinematicEngine.IsCinematicRunning() ) {
						sf::RectangleShape rs(sf::Vector2f( GetSize().x, GetSize().y) );
						rs.setPosition(gameView.getCenter().x-GetSize().x/2, gameView.getCenter().y-GetSize().y/2);
						rs.setFillColor(sf::Color(0, 0, 0, 140));
						Draw(rs);
					}
				}

				if( upToUI ) {
					renderWindow.setView(renderWindow.getDefaultView());
				}

				// Only draw if it's relevent to a cinematic (assuming one's up)
				if (!cinematicEngine.IsCinematicRunning() || it->second->cinematicEntity)
					it->second->Draw();
			}

			renderWindow.setView(renderWindow.getDefaultView()); // make sure we are using a static view

			if(GetOption("MinimalUI") == "Disabled") DrawLogMessages();

			// Big messages [MOVE TO METHOD]
			if( bigMessages.size() > 0 && bigMessageTimer.getElapsedTime().asSeconds() > 1.5 ) {
				AddEntity( new BigMessage(*this, bigMessages[0]), 1001, true );
				bigMessages.pop_front();
				bigMessageTimer.restart();
			}

			DrawSubtitles();

			// Draw score [MOVE TO METHOD]
			std::ostringstream outs;
			outs.imbue(std::locale("")); // For commas
			outs << std::fixed << currentPlayerScore << " qB";
			sf::Text n( outs.str(), FindFont("Action_Force.ttf", 35), 35 );
			n.setPosition( GetSize().x/2-n.getLocalBounds().width/2, 0 );
			n.setColor( sf::Color(255, 255, 255, 200) );
			if(inGame && !cinematicEngine.IsCinematicRunning() ) Draw(n);


			// Draw minimap
			if(inGame && !cinematicEngine.IsCinematicRunning() ) DrawMinimap();

			// If we're about to change levels, show loading text before the game freezes
			if( LevelChanged() && inGame && !cinematicEngine.IsCinematicRunning() ) {
				sf::Text n( "Loading...", FindFont("Action_Force.ttf", 60), 60 );
				n.setPosition( 0, 0 );
				n.setColor( sf::Color(255, 255, 255, 200) );
				Draw(n);
			}

			renderWindow.setView(gameView); // Back to the normal view

			renderWindow.display();

			PerformCameraMovement();

			// Destroy entities added for removal
			ExecuteDeletionQueue();

			pastDeltaClock = deltaClock.getElapsedTime().asSeconds();
		}
		renderWindow.close();
	} catch ( const std::exception &e ) {
		std::cout << "Unhandled exception: " << e.what() << std::endl;
	} catch ( ... ) {
		std::cout << "Non-standard unhandled exception!" << std::endl;
	}

	renderWindow.close();
	system("pause");
}

void App2D::Draw(const sf::Drawable &object) {
	renderWindow.draw( object );
}

/*const sf::Input &App2D::GetInput() {
	return renderWindow.GetInput();
}*/

void App2D::Close() {
	isClosing = true;
}

sf::Vector2i App2D::GetSize() const {
	return sf::Vector2i(renderWindow.getSize().x, renderWindow.getSize().y);
}

EventHandler &App2D::GetEventHandler() {
	return eventHandler;
}

float App2D::GetFrameTime() const {
	return pastDeltaClock;
}

JsonPool &App2D::GetJsonPool() {
	return jsonPool;
}

long long App2D::nextId() {
	return ++currentId;
}

long long App2D::nextViewId() {
	return nextId();//return 1e9 + ++currentViewId;
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
		sf::Text n( sf::String(currentSubtitles), FindFont("Temp7c.ttf", 15), 15 );
		n.setPosition( GetSize().x/2 - n.getLocalBounds().width/2, GetSize().y-100 );
		n.setColor( sf::Color(255, 255, 255, 255) );

		sf::RectangleShape r( sf::Vector2f(n.getLocalBounds().width, n.getLocalBounds().height));
		r.setFillColor(sf::Color::Black);
		r.setPosition(n.getPosition());
		Draw(r);
		Draw(n);
	}

	// Dialogue has to be checked for 'stoppage' every frame
	for( std::vector<sf::Sound*>::iterator it = playingSounds.begin(); it != playingSounds.end(); ++it ) {
		if( *it == currentDialoguePtr && (*it)->getStatus() == sf::Sound::Stopped) {
			currentDialogue = "";
			currentDialoguePtr = nullptr;
			delete (*it);
			playingSounds.erase(it);
			break;
		}
	}
}

void App2D::DisplayBigMessage(std::string message) {
	bigMessages.push_back(message);
}

void App2D::CreateParticles() 
{
	// Make binary particles for effect
	if(binaryReplaceTimer.getElapsedTime().asSeconds() > 0.05f  * EvaluateOption("ParticleDensity"))	 {
		AddEntity( new BinaryParticle(*this), 1 );
		binaryReplaceTimer.restart();
	}
}

// Some manager functions to avoid resource duplicates

sf::Font &App2D::FindFont( std::string dir, int size ) {
	dir = "../media/font/" + dir;
	if( fontMap.find(dir) == fontMap.end() ) {
		std::cout << "New font: " << dir << std::endl;
		if(!fontMap[dir].loadFromFile(dir)) {
			throw std::exception(std::string(dir + " could not be loaded").c_str());
		}
	}
	return fontMap.find(dir)->second;
}

sf::Texture &App2D::FindTexture( std::string dir ) {
	dir = "../media/image/" + dir;
	if( textureMap.find(dir) == textureMap.end() ) {
		std::cout << "New image: " << dir << std::endl;
		if(!textureMap[dir].loadFromFile(dir)) {
			throw std::exception(std::string(dir + " could not be loaded").c_str());
		}
	}
	return textureMap.find(dir)->second;
}


// MEMORY LEAK HERE
// Shaders are never deleted. Not that it really matters (there's only 2) but should probably get around to it
sf::Shader &App2D::FindShader( std::string dir ) {
	dir = "../media/shader/" + dir;
	if( shaderMap.find(dir) == shaderMap.end() ) {
		std::cout << "New shader: " << dir << std::endl;
		shaderMap[dir] = new sf::Shader;
		if(!shaderMap[dir]->loadFromFile(dir, sf::Shader::Fragment)) {
			throw std::exception(std::string(dir + " could not be loaded").c_str());
		}
		shaderMap[dir]->setParameter("framebuffer", sf::Shader::CurrentTexture);
	}
	return *shaderMap.find(dir)->second;
}



sf::Sound *App2D::PlaySound( std::string sound, bool loop, bool useDist, int x, int y, int vol ) {
	sound = "../media/sound/" + sound;
	if( soundMap.find(sound) == soundMap.end() ) {
		if(!soundMap[sound].loadFromFile(sound)) {
			throw std::exception(std::string(sound + " could not be loaded").c_str());
		}
	}

	sf::Sound *newSound = new sf::Sound(soundMap[sound]);
	playingSounds.push_back( newSound );
	newSound->setLoop(loop);

	if( !useDist ) { // Use distance from camera as volume
		newSound->setVolume(vol);
	} else {
		float dist = sqrt(powf(x-gameView.getCenter().x, 2) + powf(y-gameView.getCenter().y, 2));
		float v = 100-(dist/20.0);
		newSound->setVolume( v > 0 ? v : 0 );
	}
	playingSounds.back()->play();

	// Get rid of all the sounds that have stopped playing
	bool isErasing = true; // Boolean required 'cause of iterator invalidation
	while(isErasing) {
		isErasing = false;
		for( std::vector<sf::Sound*>::iterator it = playingSounds.begin(); it != playingSounds.end(); ++it ) {
			if( (*it)->getStatus() == sf::Sound::Stopped) {
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
		if( currentMusic ) currentMusic->stop();
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

	sf::RectangleShape box( sf::Vector2f(sx-1, sy-1) );
	box.setPosition(GetSize().x-sx, GetSize().y-sy);
	box.setFillColor( sf::Color(0, 0, 0, 150) );
	box.setOutlineColor(sf::Color(180, 180, 180) );
	box.setOutlineThickness(1);
	Draw(box);
	sf::CircleShape s(3, 3);

	for ( EntityMap::iterator it = entities.begin(); it != entities.end(); ++it ) {
		if( !it->second->type.empty() && it->second->isEnemy ) {
			s.setFillColor( sf::Color( 255, 0, 0 ) );
			s.setPosition( GetSize().x - sx + (abs(field.bleft.x)+it->second->x)*fx,
						   GetSize().y - sy + (abs(field.bleft.y)+it->second->y)*fy );
			Draw(s);
		} else if ( !it->second->type.empty() && it->second->type == "localplayer" ) {
			s.setFillColor( sf::Color( 0, 255, 0 ) );
			s.setPosition( GetSize().x - sx + (abs(field.bleft.x)+it->second->x)*fx,
				GetSize().y - sy + (abs(field.bleft.y)+it->second->y)*fy );
			Draw(s);
		} else if ( !it->second->type.empty() && it->second->type == "warper") {
			s.setFillColor( sf::Color( 0, 0, 255 ) );
			if( it->second->displayName == ".." ) s.setFillColor( sf::Color( 255, 255, 0 ) );
			s.setPosition( GetSize().x - sx + (abs(field.bleft.x)+it->second->x)*fx,
				GetSize().y - sy + (abs(field.bleft.y)+it->second->y)*fy );
			Draw(s);
		} else if( !it->second->type.empty() && InField(it->second.get()) ) {
			s.setFillColor( sf::Color( 128, 128, 128 ) );
			s.setPosition( GetSize().x - sx + (abs(field.bleft.x)+it->second->x)*fx,
				GetSize().y - sy + (abs(field.bleft.y)+it->second->y)*fy );
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
	ofs.open("options.json", std::ios::out | std::ios::trunc);

	if( !ofs.is_open() ) {
		throw std::exception("Couldn't open options file for writing!");
	}

	Json::Value root;

	std::map< std::string, std::string >::iterator it;
	for( it = gameOptions.begin(); it != gameOptions.end(); ++it ) {
		root[it->first] = it->second;
	}


	ofs << root;

	ofs.close();
}

void App2D::LoadOptions() {
	Json::Value root = jsonPool.GetRootNode("options.json");

	std::vector< std::string > memberNames = root.getMemberNames();
	for( int i = 0; i != memberNames.size(); ++i ) {
		SetOption( memberNames[i], root[ memberNames[i] ].asString() );
	}
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
	currentPlayerScore = 0;
	ExecuteDeletionQueue();
	currentPath = GetOption("InitialDirectory");
}

void App2D::ReEnterGame() {
	Json::Value root = jsonPool.GetRootNode("saves/!header.json");
	if( IsLastSave() ) {
		currentPath = root["CurrentPath"].asString();
		currentPlayerScore = root["PlayerScore"].asInt();
		std::cout << "Loaded savefile in directory: " << currentPath << std::endl;
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
			AddEntity( new DefinedEntity( *this, "warper", rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, dirs[i].filename().string(), scale ), 11 );
		} else {
			DisplayMessage("\'" + dirs[i].filename().string() + "\' is an innaccessible subdirectory - not spawning wormhole.");
		}
	}
	// Add a warper that allows the player to go up a directory
	AddEntity( new DefinedEntity( *this, "warper", rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, ".." ), 10 );

	std::sort( files.begin(), files.end(), SizeCmp() );

	// For every file, if it ain't destroyed, search the database for a suitable enemy

	for( int i = 0; i != files.size(); ++i ) {

		if( CheckFile(files[i].filename().string()) != "destroyed" ) {

			Json::Value root = jsonPool.GetRootNode("../media/entity/!enemytypes.json");
			Json::Value::Members enemies = root.getMemberNames();
			int largestEnemySize = 0;
			std::string bestEnemyType;
			for( int j = 0; j != enemies.size(); ++j ) {
				int mySize = root[enemies[j]]["MinFileSize"].asInt();
				if( mySize <= file_size(files[i]) && mySize >= largestEnemySize) {
					largestEnemySize = mySize;
					bestEnemyType = enemies[j];
				}
			}
			
			if( bestEnemyType.empty() ) {
				std::cout << "Couldn't suit file: " << files[i].filename().string() << " at " << file_size(files[i]) << " to enemy." << std::endl;
			} else {
				AddEntity( new DefinedEntity( *this, bestEnemyType, rand()%gameSize-gameSize/2, rand()%gameSize-gameSize/2, sf::Vector2f(), 0, false, files[i].filename().string() ), 10 );
			}
		}
	}


	AddEntity( new DefinedEntity( *this, "spawner" ), 9 );

	Json::Value &root = jsonPool.GetRootNode("saves/!header.json");

	root["CurrentPath"] = currentPath;
	root["PlayerScore"] = currentPlayerScore;

	jsonPool.WriteToFile("saves/!header.json");

	int r = rand()%8;
	if( r == 0 ) PlayDialogue("lchange1.ogg");
	if( r == 1 ) PlayDialogue("lchange2.ogg");
	if( r == 2 ) PlayDialogue("lchange3.ogg");
	if( r == 3 ) PlayDialogue("lchange4.ogg");
}

void App2D::RespawnPlayerIfDead() {
	// Make sure there is a player
	if( playerDeathTimer.getElapsedTime().asSeconds() > 3.0f && CountEntityOfType("localplayer") == 0) {
		playerName = GetOption("PlayerName");
		AddEntity( new DefinedEntity( *this, "localplayer", 150, 150, sf::Vector2f(), 0, true, playerName ), 10 );
	}
}

void App2D::PerformCameraMovement() {
	//Camera controls
	if( following ) {
		cdelta.x = toFollow->x-gameView.getCenter().x;
		cdelta.y = toFollow->y-gameView.getCenter().y;
	}
	// Camera slowdown
	cdelta.x += (-cdelta.x*GetFrameTime());
	cdelta.y += (-cdelta.y*GetFrameTime());
	float cSpd = 5; // Speed of camera following
	gameView.move(cdelta.x*GetFrameTime()*cSpd, cdelta.y*GetFrameTime()*cSpd);
}

void App2D::DrawGameField() {
	sf::VertexArray f(sf::LinesStrip, 5);
	f[0].position = sf::Vector2f( field.bleft.x, field.bleft.y ); f[0].color = sf::Color(5, 5, 255);
	f[1].position = sf::Vector2f( field.bleft.x , field.tright.y ); f[1].color = sf::Color(5, 5, 255);
	f[2].position = sf::Vector2f( field.tright.x , field.tright.y ); f[2].color = sf::Color(5, 5, 255);
	f[3].position = sf::Vector2f( field.tright.x , field.bleft.y ); f[3].color = sf::Color(5, 5, 255);
	f[4].position = sf::Vector2f( field.bleft.x, field.bleft.y ); f[4].color = sf::Color(5, 5, 255);

	Draw(f);
}

void App2D::DrawLogMessages() {
	// Draw messages, scrolling them up when new ones arrive (if minimal UI is disabled)
	for( int i = messageList.size()-maxMessages, j = 0; i != messageList.size(); ++i, ++j ) {
		sf::Text n( sf::String(messageList[i]), FindFont("BlackWolf.ttf", 12), 12 );
		n.setPosition( 0, GetSize().y-maxMessages*15+j*15-5 );
		n.setColor( sf::Color(255, 255, 255, 200) );
		Draw(n);
	}
}

// Does a previous savefile exist?
bool App2D::IsLastSave() {
	std::ifstream ifs;
	ifs.open("saves/!header.json", std::ios::in);
	if( ifs.is_open() ) {
		ifs.close();
		return true;
	}
	return false;
}

void App2D::DrawPostProcessingEffects() {
	sf::Texture t;
	if(!t.create(GetSize().x, GetSize().y)) throw std::exception("couldn't create postprocessing texture!");

	while( postProcessingStack.size() > 0 ) {
		std::pair< std::string, sf::Vector2f > info( postProcessingStack.front() );
		sf::Shader &fx = FindShader(info.first);
		fx.setParameter("position", info.second.x, info.second.y);

		t.update(renderWindow);
		sf::Sprite s;
		s.setTexture(t);
		s.setPosition(gameView.getCenter().x-GetSize().x/2, gameView.getCenter().y-GetSize().y/2);


		sf::RenderStates st;
		st.shader = &fx;
		renderWindow.draw(s, st);

		postProcessingStack.pop();
	}
}