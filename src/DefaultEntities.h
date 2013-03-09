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

		strFPS.SetFont(app.FindFont("BlackWolf.ttf", 12)); strFPS.SetSize(12);
		strObjects.SetFont(app.FindFont("BlackWolf.ttf", 12)); strObjects.SetSize(12);

		// Set text colours
		strFPS.SetColor( sf::Color(200, 200, 255, 200 ) );
		strObjects.SetColor( sf::Color(200, 200, 255, 200 ) );
	}

	~FPSMeter() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		// Update FPS
		oss.str("");
		oss << int(1.f/delta) << " FPS";
		strFPS.SetText( oss.str() );

		// Update object count
		oss.str("");
		oss << app.GetObjectCount() << " Objects";
		strObjects.SetText( oss.str() );
		strObjects.SetPosition( app.GetSize().x - strObjects.GetCharacterPos(oss.str().size()).x, 0 );
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
	sf::String strFPS, strObjects;
};

// Ugly class to show splash screens
class SplashScreen : public Entity {
public:
	SplashScreen( App2D &a ) : Entity(a) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &SplashScreen::onStep);

		// Load and init splash images in middle of screen
		schs.SetImage(app.FindImage("SchnommusLogo.png"));
		mets.SetImage(app.FindImage("MetanactLogo.png"));

		met.SetSmooth(false);
		sch.SetSmooth(false);

		mets.SetCenter( mets.GetSize().x/2, mets.GetSize().y/2 );
		schs.SetCenter( schs.GetSize().x/2, schs.GetSize().y/2 );

		mets.SetPosition( app.GetSize().x/2, app.GetSize().y/2 );
		schs.SetPosition( app.GetSize().x/2, app.GetSize().y/2 );

		mets.SetColor(sf::Color(255, 255, 255, 0));
		schs.SetColor(sf::Color(255, 255, 255, 0));

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
	sf::Image met, sch;
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
		sf::Vector2f v = app.renderWindow.ConvertCoords((rand()%(app.GetSize().x+sDelta))-sDelta/2, (rand()%(app.GetSize().y+sDelta))-sDelta/2);
		x = v.x;
		y = v.y;
		fScale = 3/float(rand()%10+1);
		s.SetFont(app.FindFont("Temp7c.ttf"));
		s.SetSize((10/fScale+7 > 20 ? 20 : 10/fScale+7));
		s.SetText((rand()%2)?"0":"1");
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
		s.SetColor( sf::Color(50, 255, 50, alpha ) );
		s.SetPosition(x, y);
		return true;
	}

	virtual void Draw() {
		// Draw the text
		app.Draw(s);
	};

private:
	CppEventHandler hStepEvent;
	sf::String s; float alpha, fScale; bool gd;
};

class BigMessage : public Entity {
public:
	BigMessage( App2D &a, std::string message ) : Entity(a) {
		// Register events
		hStepEvent = app.GetEventHandler().stepEvent.attach(this, &BigMessage::onStep);
		alpha = 255;
		y = 80;

		s.SetFont(app.FindFont("Action_Force.ttf"));
		s.SetSize(30);
		s.SetText(message.c_str());
		x = app.GetSize().x/2 - s.GetCharacterPos(message.size()-1).x/2;
	}

	~BigMessage() {
		// Detach events
		app.GetEventHandler().stepEvent.detach( hStepEvent );
	}

	bool onStep(float delta) {
		y -= 30*delta;
		alpha -= 100*delta;
		if(alpha < 1) app.RemoveEntity(this->id);
		s.SetColor( sf::Color(255, 255, 255, alpha ) );
		s.SetPosition(x, y);
		return true;
	}

	virtual void Draw() {
		// Draw the text
		app.Draw(s);
	};

private:
	CppEventHandler hStepEvent;
	sf::String s; float alpha;
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
		s.SetImage( app.FindImage("ring.png") );
		s.SetCenter( s.GetSize().x/2, s.GetSize().y/2 );
		s.SetScale(0.6, 0.6);
		
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
		s.SetColor( sf::Color(255, 255, 255, alpha ) );
		s.Scale(1-0.5*delta, 1-0.5*delta);
		s.SetPosition(x, y);
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