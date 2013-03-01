#pragma once

#include "App2D.h"

class Tag {
public:
	Tag( Entity &entityReference ) : e(entityReference) { }
	virtual void Init() = 0;
	virtual void Step(float delta) = 0;
	virtual void Draw() = 0;
	virtual void Destroy() = 0;
	Entity &e;
};
