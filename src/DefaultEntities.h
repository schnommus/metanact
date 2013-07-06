#ifndef DEFAULTENTITIES_H
#define DEFAULTENTITIES_H

#include <sstream>
#include "App2D.h"

/// The FPSMeter class will show the application FPS and object count.
class FPSMeter : public Entity {
public:
	FPSMeter( App2D &a ) : Entity(a) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &FPSMeter::onStep);

		strFPS.setFont(app.FindFont("BlackWolf.ttf", 12)); strFPS.setCharacterSize(12);
		strObjects.setFont(app.FindFont("BlackWolf.ttf", 12)); strObjects.setCharacterSize(12);

		// Set text colours
		strFPS.setColor( sf::Color(200, 200, 255, 200 ) );
		strObjects.setColor( sf::Color(200, 200, 255, 200 ) );

		cinematicEntity = true;
	}

	~FPSMeter() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		// Update FPS
		oss.str("");
		oss << int(1.f/delta) << " FPS";
		strFPS.setString( oss.str() );

		// Update object count
		oss.str("");
		oss << app.GetObjectCount() << " Objects";
		strObjects.setString( oss.str() );
		strObjects.setPosition( app.GetSize().x - strObjects.getLocalBounds().width, 0 );
		return true;
	}

	virtual void Draw() {
		if( app.GetOption("MinimalUI") == "Disabled" ) {
			// Draw the text (Only if minimal UI disabled)
			app.Draw(strFPS);
			app.Draw(strObjects);
		}
	};

private:
	CppEventHandler hStepEvent;
	std::ostringstream oss;
	sf::Text strFPS, strObjects;
};

// Ugly class to show splash screens
class SplashScreen : public Entity {
public:
	SplashScreen( App2D &a ) : Entity(a) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &SplashScreen::onStep);

		// Load and init splash images in middle of screen
		schs.setTexture(app.FindTexture("SchnommusLogo.png"));
		mets.setTexture(app.FindTexture("MetanactLogo.png"));

		mets.setOrigin( mets.getLocalBounds().width/2, mets.getLocalBounds().height/2 );
		schs.setOrigin( schs.getLocalBounds().width/2, schs.getLocalBounds().height/2 );

		mets.setPosition( app.GetSize().x/2, app.GetSize().y/2 );
		schs.setPosition( app.GetSize().x/2, app.GetSize().y/2 );

		mets.setColor(sf::Color(255, 255, 255, 0));
		schs.setColor(sf::Color(255, 255, 255, 0));

		// Null some state values
		alpha = 0;
		f = s = c = false;
	}

	~SplashScreen() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta);

	virtual void Draw() {
		app.Draw(mets);
		app.Draw(schs);
	};

private:
	CppEventHandler hStepEvent;
	sf::Sprite mets, schs;
	float alpha; // Stores alpha
	bool f, s, c; // Finished displaying this image, second image active, complete
};

class BinaryParticle : public Entity {
public:
	BinaryParticle( App2D &a ) : Entity(a) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &BinaryParticle::onStep);
		alpha = 2; gd = false;
		int sDelta = 1000;
		sf::Vector2f v = app.renderWindow.convertCoords(sf::Vector2i( (rand()%(app.GetSize().x+sDelta))-sDelta/2, (rand()%(app.GetSize().y+sDelta))-sDelta/2) );
		x = v.x;
		y = v.y;
		fScale = 3/float(rand()%10+1);
		s.setFont(app.FindFont("Temp7c.ttf"));
		s.setCharacterSize((10/fScale+7 > 20 ? 20 : 10/fScale+7));
		s.setString((rand()%2)?"0":"1");
		cinematicEntity = true;
	}

	~BinaryParticle() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		x += app.cdelta.x*delta*fScale;
		y += app.cdelta.y*delta*fScale;

		if( !gd ) alpha += 100*delta;
		else alpha -= 100*delta;
		if( alpha > 150 ) gd = true;
		if(alpha < 1) app.RemoveEntity(this->id);
		s.setColor( sf::Color(50, 255, 50, alpha ) );
		s.setPosition(x, y);
		return true;
	}

	virtual void Draw() {
		// Draw the text
		app.Draw(s);
	};

private:
	CppEventHandler hStepEvent;
	sf::Text s; float alpha, fScale; bool gd;
};

class ImageParticle : public Entity {
public:
	ImageParticle( App2D &a, std::string imageName, int discolouration = 1, float scale = 1, float movementScale = 1, bool forever = false, bool overridePath = false )
		: Entity(a), mScale(movementScale), isForever(forever) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &ImageParticle::onStep);
		alpha = 2; gd = false;
		int sDelta = 1000;
		sf::Vector2f v = app.renderWindow.convertCoords(sf::Vector2i( (rand()%(app.GetSize().x+sDelta))-sDelta/2, (rand()%(app.GetSize().y+sDelta))-sDelta/2) );
		x = v.x;
		y = v.y;
		cinematicEntity = true;
		if( isForever ) {
			alpha = 100;
			int gameSize = app.field.tright.x*2;
			x = rand()%gameSize-gameSize/2;
			y = rand()%gameSize-gameSize/2;
			s.setRotation(rand()%360);
			cinematicEntity = false;
		}

		fScale = scale/float(rand()%3+1);
		s.setTexture( app.FindTexture(imageName, overridePath) );
		s.setScale( fScale, fScale );
		s.setColor( sf::Color(255-rand()%discolouration, 255-rand()%discolouration, 255-rand()%discolouration, 255 ) );
		
	}

	~ImageParticle() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		x += mScale*(app.cdelta.x*delta)/fScale;
		y += mScale*(app.cdelta.y*delta)/fScale;

		if( !isForever ) {
			if( !gd ) alpha += 100*delta;
			else alpha -= 100*delta;
			if( alpha > 230 ) gd = true;
			if(alpha < 1) app.RemoveEntity(this->id);
		}

		sf::Color myColour = s.getColor(); myColour.a = alpha;
		s.setColor( myColour );
		s.setPosition(x, y);
		return true;
	}

	virtual void Draw() {
		// Draw the text
		app.Draw(s);
	};

private:
	CppEventHandler hStepEvent;
	sf::Sprite s; float alpha, fScale, mScale; bool gd, isForever;
};


class BigMessage : public Entity {
public:
	BigMessage( App2D &a, std::string message ) : Entity(a), msg(message) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &BigMessage::onStep);
		alpha = 0;
		y = 150;
		down = false;

		std::cout << "Big message: " << msg << std::endl;

		s.setFont(app.FindFont("Action_Force.ttf"));
		s.setCharacterSize(30);
		s.setString(msg.c_str());
		x = app.GetSize().x/2 - s.getLocalBounds().width/2;

		app.PlaySound("message.ogg");
	}

	~BigMessage() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		return true;
	}

	virtual void Draw() {
		y -= 20*app.GetFrameTime();
		x = app.GetSize().x/2 - s.getLocalBounds().width/2;
		if(down) {
			alpha -= 100*app.GetFrameTime();
			if(alpha < 1) app.RemoveEntity(this->id);
		} else
			alpha += 200*app.GetFrameTime();

		if( !down && alpha > 200 ) down = true;

		s.setColor( sf::Color(255, 255, 255, alpha ) );
		s.setPosition(x, y);

		// Draw the text
		app.Draw(s);
	};

private:
	CppEventHandler hStepEvent;
	sf::Text s; float alpha; bool down; std::string msg;
};

class CreditMessage : public Entity {
public:
	CreditMessage( App2D &a, std::string message ) : Entity(a), msg(message) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &CreditMessage::onStep);
		alpha = 0;
		y = 20;
		cinematicEntity = true;
		down = false;

		std::cout << "Credits: " << msg << std::endl;

		s.setFont(app.FindFont("oAction_Force.ttf"));
		s.setCharacterSize(40);
		s.setString(msg.c_str());
		x = 2550;
	}

	~CreditMessage() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		return true;
	}

	virtual void Draw() {
		//y -= 20*app.GetFrameTime();
		if(!down) 
			x -= 1200*app.GetFrameTime();
		else
			x -= 70*app.GetFrameTime();
		if(down) {
			alpha -= 50*app.GetFrameTime();
			if(alpha < 1) app.RemoveEntity(this->id);
		} else
			alpha += 100*app.GetFrameTime();

		if( !down && alpha > 200 ) down = true;

		s.setColor( sf::Color(255, 255, 255, alpha ) );
		s.setPosition(x, y);

		// Draw the text
		app.Draw(s);
	};

private:
	CppEventHandler hStepEvent;
	sf::Text s; float alpha; bool down; std::string msg;
};


class SmokeParticle : public Entity {
public:
	SmokeParticle( App2D &a, int px, int py, float strength_v ) : Entity(a) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &SmokeParticle::onStep);
		alpha = 2; gd = false;
		x = px;
		y = py;
		strength = strength_v;
		fScale = 15;
		s.setTexture( app.FindTexture("ring.png") );
		s.setOrigin( s.getLocalBounds().width/2, s.getLocalBounds().height/2 );
		s.setScale(0.6, 0.6);

		cinematicEntity = true;
		
	}

	~SmokeParticle() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		if( !gd ) alpha += 300*delta;
		else alpha -= 100*delta;
		if( alpha > strength ) gd = true;
		if(alpha < 1) app.RemoveEntity(this->id);
		s.setColor( sf::Color(255, 255, 255, alpha ) );
		s.scale(1-0.5*delta, 1-0.5*delta);
		s.setPosition(x, y);
		return true;
	}

	virtual void Draw() {
		// Draw the text
		app.Draw(s);
	};

private:
	CppEventHandler hStepEvent;
	sf::Sprite s; float alpha, fScale, strength; bool gd;
};

#endif