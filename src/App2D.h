#ifndef APP2D_H
#define APP2D_H

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>

#include <iostream>
#include <string>
#include <exception>
#include <vector>
#include <map>
#include <utility>
#include <memory>

#include "Entity.h"
#include "EventHandler.h"
#include "CinematicEngine.h"

using std::tr1::shared_ptr;

struct PlayingField {
	sf::Vector2f bleft, tright;
};

/// Holds most of the things a 2D application needs to function
class App2D {
public:

	/// Initializes the application - takes window settings etc. All but the first parameter are optional.
	/// \param useVSync : True if you want VSync enabled. If using physics you need this enabled.
	/// \param useCurrentResolution : Set to true if you want to use the user's currently set resolution
	/// \param title : The window title, as shown in the title bar and in task manager
	/// \param fullscreen : Set to true for a fullscreen application
	/// \param pname : The player's name
	/// \param size_x : The x-axis resolution (When useCurrentResolution == false)
	/// \param size_y : The y-axis resolution (When useCurrentResolution == false)
	App2D( bool useVSync = true,
		   bool useCurrentResolution = false,
		   const std::string &title = "UntitledWindow",
		   bool fullscreen = false,
		   std::string pname = "noname",
		   int size_x = 800,
		   int size_y = 600 );

	/// All entities are destroyed here
	~App2D();
	
	/// When called, the application will open a window, call the specified creationFunction and begin the event loop.
	void Run();

	void DrawLogMessages();

	void DrawGameField();

	void PerformCameraMovement();

	void RespawnPlayerIfDead();

	void CreateParticles();

	void DrawSubtitles();

	bool IsLastSave();

	/// Use this to draw your entities' drawables when it needs to (When your entities' virtual Draw() method is called)
	/// \param object : The drawable object to draw. The draw order will depend on the entities' draw order specified on creation.
	void Draw( const sf::Drawable &object );

	/// Get SFML's internal input class. Use it to check if a key is down, mouse position etc.
	/// \return The input class.
	const sf::Input &GetInput();

	/// Closes the application.
	void Close();

	/// Adds an entity to the manager. On clients, this does nothing
	/// \param e : A freshly new'ed Entity off the stack. Destruction is internally taken care of.
	/// \param layer : The draw layer, changes the order in which calls to Entity::Draw() overrides are called. Higher is on top, >1000 is UI
	long long AddEntity( Entity *e , int layer, bool isViewEntity = false);

	Entity *GetEntityWithId( long long idToGet );

	void DrawMinimap();

	bool InField( Entity *e );

	sf::Sound *PlaySound( std::string sound, bool loop = false, bool useDist = false, int x=0, int y=0, int vol=100 );

	void SetMusic(std::string music);

	void PlayDialogue(std::string filename);
	
	sf::Image &FindImage( std::string dir );
	sf::Font &FindFont( std::string dir, int size=30 );
	sf::PostFX &FindShader( std::string dir );

	int CountEntityOfType( std::string type );

	std::string GetOption( std::string type );
	void SetOption( std::string type, std::string data );
	float EvaluateOption( std::string type);
	void LoadOptions();
	void SaveOptions();

	void ReleaseFile( std::string subnamw, std::string message );
	std::string CheckFile( std::string subname );

	void WipeCurrentGame();
	void ReEnterGame();

	// Display a message in the message scroller
	void DisplayMessage(std::string message);

	// Display a fading message at the top of the screen
	void DisplayBigMessage(std::string message);

	void LoadLevel();

	bool LevelChanged();

	///Entity following (by camera)
	void FollowEntity( Entity &e );
	void StopFollowing();

	//Delete entites added to deletion queue
	void ExecuteDeletionQueue();

	/// Remove an entity from the manager.
	/// \param id : The id of the entity to remove. For deleting itself, an entity can call this with it's own id.
	void RemoveEntity( long long id );

	/// Get the number of objects (Entities) in the application.
	/// \return The number of objects
	int GetObjectCount();

	/// Get the application event handler. Used for attaching entities to events.
	/// \return The event handler
	EventHandler &GetEventHandler();

	/// Gets the size of the application window.
	/// \return The size of the application window as a 2D vector.
	sf::Vector2i GetSize() const;

	/// Get the time since last frame, in seconds.
	/// \return The frame time in seconds.
	float GetFrameTime() const;

	typedef std::multimap<int, shared_ptr<Entity> > EntityMap;

private:
	App2D( const App2D& );
	App2D &operator=( const App2D& );

	long long nextId();
	long long nextViewId();

	std::vector<int> idRemovalList;

	long long currentId;
	long long currentViewId;
	
	bool vSync;

	sf::Sound *currentMusic;

	EventHandler eventHandler;
public:
	CinematicEngine cinematicEngine;

	bool isClosing;
	sf::RenderWindow renderWindow;

	sf::View gameView;

	PlayingField field;

	EntityMap entities;

	std::string playerName;
	std::vector<std::string> messageList;
	int maxMessages;

	// For camera following
	bool following;
	Entity *toFollow;
	sf::Vector2f cdelta;

	std::map< std::string, sf::SoundBuffer > soundMap;
	std::vector< sf::Sound* > playingSounds;

	std::map< std::string, std::string > gameOptions;

	std::map< std::string, sf::Image > imageMap;

	std::map< std::string, sf::PostFX > shaderMap;

	std::map< std::string, sf::Font > fontMap;

	std::string oldpath;
	std::string currentPath;

	std::string currentDialogue;
	std::string currentSubtitles;
	sf::Sound *currentDialoguePtr;

	bool inGame;

	bool currentLevelUnlocked;

	sf::Clock playerDeathTimer;
	sf::Clock binaryReplaceTimer;
};

// Include the default entities.
#include "DefaultEntities.h"

#endif