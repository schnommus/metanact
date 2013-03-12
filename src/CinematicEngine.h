#ifndef CINEMATICENGINE_H
#define CINEMATICENGINE_H

#include "App2D.h"
#include <vector>
#include <map>
#include <boost/smart_ptr.hpp>

class CinematicEngine {
public:
	CinematicEngine( App2D &a ) : app(a), cinematicRunning(false) { }

	void RunCinematic( std::string filename );

	void UpdateCinematic();

	bool IsCinematicRunning() { return cinematicRunning; }

	App2D &app;
	bool cinematicRunning;
	sf::Clock masterClock;

	// Maps event strings to their times
	std::map< std::string, float > eventMap;

	// Maps named entities to their respective ids
	std::map< std::string, long long > namedEntityMap;

	//I.E
	// 6.2 NAMED_ENTITY derp1 0 100
	// 6.2 MOVE_TO derp1 -100 0 9.5
	// 9.6 DELETE derp1
	// 10.0 END_CINEMATIC
};

#endif