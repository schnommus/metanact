#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

class App2D;

/// Base class for custom Entities. Noncopyable.
class Entity {
public:
	/// Must construct this class with an application instance. Do it in your classes' constructor.
	/// \param a : The application instance.
	Entity( App2D &a ) : app(a) { }

	virtual ~Entity() {}
	
	/// Called in draw order - override for drawing your entity 
	virtual void Draw() = 0;

	/// The entities' id.
	long long id;

	/// Entity position
	float x, y, rotation, scale;

	/// Entity velocity
	sf::Vector2f vel;

	/// Entity type
	std::string type;

	/// Application it is linked to
	App2D &app;

	/// Internal shape
	std::vector<sf::Shape> sv;

	/// Is the entity warping?
	bool inWarp;
	short warpId;
	short warpParent;

	/// Only active when level unlocked?
	bool onUnlockOnly;

	/// Entity health (if it has any)
	int health;

	// Will be drawn if it contains something and is a DefinedEntity
	std::string displayName;

	// Directory of the entities' associated image
	std::string imageDir;
	sf::Sprite imageSprite;

	sf::Clock deletionTimeout;
protected:
	Entity( const Entity& );
	Entity &operator=( const Entity& );
};

#endif