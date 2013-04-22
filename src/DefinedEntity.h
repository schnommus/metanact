#ifndef DEFINED_ENTITY_H
#define DEFINED_ENTITY_H

#include "App2D.h"
#include <utility>
#include <vector>
#include <fstream>
#include <boost\shared_ptr.hpp>
#include "Tag.h"
#include "Tags.h"


class GravityData {
public:
	GravityData( int xp, int yp, int rad, float s, float d ) : x(xp), y(yp), radius(rad), strength(s), delta(d) { }
	int x, y, radius;
	float strength, delta;
};

class DefinedEntity : public Entity {
public:
	DefinedEntity( App2D &app, std::string type, int xpos=0, int ypos=0, sf::Vector2f sVel = sf::Vector2f(), float sRotation = 0, bool iw = false, std::string dispName = "", float mscale = 1.0 );

	bool onStep( float delta );

	virtual void Draw();

	~DefinedEntity();

private:
	CppEventHandler hStepEvent;
	std::vector< boost::shared_ptr<Tag> > tagList;
	bool drawOverride;
};

#endif