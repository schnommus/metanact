#ifndef DEFINED_ENTITY_H
#define DEFINED_ENTITY_H

#include "App2D.h"
#include <utility>
#include <vector>
#include <fstream>
#include <boost\shared_ptr.hpp>
#include "Tag.h"
#include "Tags.h"

// Class to read line-shape definitions
class LineReader {
public:
	static std::vector<sf::Shape> Read ( const char *file ) {
		std::ifstream ifs;
		ifs.open(file, std::ios::in);

		if( !ifs.is_open() ) {
			throw std::exception(std::string(std::string("Invalid entity shape definition name: ") + std::string(file)).c_str());
		}

		std::vector< std::pair< sf::Vector2f, sf::Vector2f > > sDef;

		float cx, cy; // Get object centre
		ifs >> cx >> cy;

		float r, g, b; // Get object colour
		ifs >> r >> g >> b;

		std::vector<sf::Shape> shapeVector;
		while( !ifs.eof() ) {
			float x1, y1, x2, y2;
			ifs >> x1 >> y1 >> x2 >> y2;
			shapeVector.push_back( sf::Shape::Line(x1, y1,
							 x2, y2, 1, sf::Color(r, g, b) ) );
			shapeVector[shapeVector.size()-1].SetCenter(cx, cy);
		}
		return shapeVector;
	}
};

class GravityData {
public:
	GravityData( int xp, int yp, int rad, float s, float d ) : x(xp), y(yp), radius(rad), strength(s), delta(d) { }
	int x, y, radius;
	float strength, delta;
};

class DefinedEntity : public Entity {
public:
	DefinedEntity( App2D &app, std::string type, int xpos=0, int ypos=0, sf::Vector2f sVel = sf::Vector2f(), float sRotation = 0, bool iw = false, std::string dispName = "" );

	bool onStep( float delta );

	virtual void Draw();

	~DefinedEntity();

private:
	CppEventHandler hStepEvent;
	std::vector< boost::shared_ptr<Tag> > tagList;
	bool drawOverride;
};

#endif