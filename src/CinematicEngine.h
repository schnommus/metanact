#ifndef CINEMATICENGINE_H
#define CINEMATICENGINE_H

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <boost/smart_ptr.hpp>

class App2D;

struct Movement {
	Movement(App2D &a) : app(a) { }
	App2D &app;
	long long eid;
	float beginTime, endTime;
	sf::Vector2f beginPos, endPos;
	virtual void ApplyForGlobalTime( float time ) = 0;
	virtual void EndMovement() = 0;
};

class CinematicEngine {
public:
	CinematicEngine( App2D &a ) : app(a), cinematicRunning(false) { }

	void RunCinematic( std::string filename );

	void UpdateCinematic();

	void DrawCinematicGlobal(); // For global transform and

	void DrawCinematicView(); // For view transform coordinate systems

	bool IsCinematicRunning() { return cinematicRunning; }

	App2D &app;
	bool cinematicRunning;
	sf::Clock masterClock;
	std::string longName;

	typedef std::multimap< float, std::string > EventMapType;

	// Maps event strings to their times
	EventMapType eventMap;

	// Maps named entities to their respective ids
	std::map< std::string, long long > namedEntityMap;

	std::vector< Movement* > activeMovements;

};

#endif