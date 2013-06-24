#include "CinematicEngine.h"
#include "App2D.h"
#include "DefinedEntity.h"
#include <fstream>
#include <sstream>

void CinematicEngine::RunCinematic( std::string filename ) {
	cinematicRunning = true;
	masterClock.restart();
	eventMap.clear();
	namedEntityMap.clear();

	// Cache all of the events
	std::ifstream ifs;
	ifs.open(("../media/entity/"+filename).c_str(), std::ios::in);
	if( ifs.is_open() ) {
		std::string all; getline(ifs, all);
		longName = all;
		std::cout << "Caching cinematic: " << longName << std::endl;
		while( !ifs.eof() ) {
			getline(ifs, all);
			std::string n, msg;
			if ( all.find(':') != std::string::npos) // If there's an '='
				n = all.substr(0, all.find(':')), msg = all.substr(all.find(':')+1, all.size()-1 );
			std::istringstream iss(n);
			float f; iss >> f;
			eventMap.insert( make_pair(f, msg) );
		}
		ifs.close();
	} else {
		throw std::exception(("Couldn't find cinematic: "+filename).c_str());
	}

	std::cout << "LOADED: " << longName << std::endl;
	for( EventMapType::iterator e = eventMap.begin(); e != eventMap.end(); ++e ) {
		std::cout << "At time " << e->first << " DO " << e->second << std::endl;
	}

	app.StopFollowing();
}

struct LinearMovement : public Movement {
	LinearMovement( App2D &a, float beg, float end, sf::Vector2f begp, sf::Vector2f endp, long long tid ) :
		Movement(a) {
			beginTime=beg; endTime=end; beginPos=begp; endPos=endp; eid=tid;
			Entity *e = app.GetEntityWithId( eid );
			e->x = begp.x;
			e->y = begp.y;
		}

	virtual void ApplyForGlobalTime( float time ) {
		float totalEventTime = endTime-beginTime;
		Entity *e = app.GetEntityWithId( eid );
		e->vel.x = ((endPos.x - beginPos.x)/totalEventTime)/10;
		e->vel.y = ((endPos.y - beginPos.y)/totalEventTime)/10;
	}

	virtual void EndMovement() {
		Entity *e = app.GetEntityWithId( eid );
		e->vel.x = e->vel.y = 0;
	}
};

struct SineMovement : public Movement {
	SineMovement( App2D &a, float beg, float end, sf::Vector2f begp, sf::Vector2f endp, long long tid ) :
	Movement(a) {
		beginTime=beg; endTime=end; beginPos=begp; endPos=endp; eid=tid;
		Entity *e = app.GetEntityWithId( eid );
		e->x = begp.x;
		e->y = begp.y;
	}
	float t;
	virtual void ApplyForGlobalTime( float time ) {
		float totalEventTime = endTime-beginTime;
		Entity *e = app.GetEntityWithId( eid );
		float interp = (3.14159/totalEventTime)*(time-beginTime);

		// For some reason the thing has to be multiplied by 1.572 which is about pi/2. I'm not sure why though,
		// the integral of sinx from 0 to pi is 2; so if anything I should be dividing by two. Meh; it works.

		e->vel.x = 1.572*sin(interp)*((endPos.x - beginPos.x)/totalEventTime)/10;
		e->vel.y = 1.572*sin(interp)*((endPos.y - beginPos.y)/totalEventTime)/10;
	}

	virtual void EndMovement() {
		Entity *e = app.GetEntityWithId( eid );
		e->vel.x = e->vel.y = 0;
		e->x = endPos.x; e->y = endPos.y;
	}
};

void CinematicEngine::UpdateCinematic() {
	if( cinematicRunning ) {
		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) ) cinematicRunning = false;

		app.CreateParticles();

		// Take care of movement
		for( std::vector<Movement*>::iterator it = activeMovements.begin(); it != activeMovements.end(); ++it ) {
			if( app.GetEntityWithId((*it)->eid) == 0 ) {
				activeMovements.erase(it);
				break;
			} 

			if( (*it)->endTime < masterClock.getElapsedTime().asSeconds() ) {
				(*it)->EndMovement();
				delete *it;
				activeMovements.erase(it);
				break;
			}

			(*it)->ApplyForGlobalTime( masterClock.getElapsedTime().asSeconds() );
		}

		for( EventMapType::iterator e = eventMap.begin(); e != eventMap.end(); ++e ) {
			if( e->first < masterClock.getElapsedTime().asSeconds() ) {
				// Do what e->second says to do
				std::istringstream iss(e->second);
				std::string s; iss >> s;

				if( s == "END_CINEMATIC") {
					cinematicRunning = false;
					//app.followEntity (theplayer if they exist)
				} else if (s == "PLAY_MUSIC") {
					std::string fname; iss >> fname;
					app.SetMusic(fname);
				} else if (s == "PLAY_SOUND") {
					std::string fname; iss >> fname;
					app.PlaySound(fname);
				} else if (s == "DIALOGUE") {
					std::string fname; iss >> fname;
					app.PlayDialogue(fname);
				}  else if (s == "CREDIT") {
					std::string message; std::getline(iss, message);
					app.AddEntity( new CreditMessage(app, message), 1001, true );
				} else if (s == "NAMED_ENTITY") {
					std::string etype, ename; iss >> etype >> ename;
					long long id = app.AddEntity( new DefinedEntity( app, etype ), 10 );
					namedEntityMap.insert(make_pair(ename, id));
				} else if (s == "SET_PARTICLETYPE") {
					std::string ptype; iss >> ptype;
					app.SetOption("ParticleType", ptype);
				} else if (s == "SET_POSITION") {
					std::string ename; float x, y; iss >> ename >> x >> y;
					Entity *e = app.GetEntityWithId( namedEntityMap.at(ename) );
					e->x = x;
					e->y = y;
				} else if (s == "COPY_POSITION") {
					std::string ename1, ename2; iss >> ename1 >> ename2;
					Entity *e1 = app.GetEntityWithId( namedEntityMap.at(ename1) );
					Entity *e2 = app.GetEntityWithId( namedEntityMap.at(ename2) );
					e2->x = e1->x;
					e2->y = e1->y;
				} else if (s == "DELETE") {
					std::string ename; iss >> ename;
					app.RemoveEntity( namedEntityMap.at(ename) );
					namedEntityMap.erase(ename);
				} else if (s == "CAMERA_FOLLOW") {
					std::string ename; iss >> ename;
					app.FollowEntity( *app.GetEntityWithId( namedEntityMap.at(ename) ) );
				} else if (s == "MOVE_LINEAR") {
					std::string ename; float tx, ty, etime; iss >> ename >> tx >> ty >> etime;
					Entity *e = app.GetEntityWithId( namedEntityMap.at(ename) );
					activeMovements.push_back( new LinearMovement(app, masterClock.getElapsedTime().asSeconds(), etime, sf::Vector2f(e->x, e->y), sf::Vector2f(tx, ty), e->id ) );
				} else if (s == "MOVE_SINE") {
					std::string ename; float tx, ty, etime; iss >> ename >> tx >> ty >> etime;
					Entity *e = app.GetEntityWithId( namedEntityMap.at(ename) );
					activeMovements.push_back( new SineMovement(app, masterClock.getElapsedTime().asSeconds(), etime, sf::Vector2f(e->x, e->y), sf::Vector2f(tx, ty), e->id ) );
				}

				// Then we don't have to worry about it any more
				eventMap.erase(e);
				break;
			}
		}

		if (!cinematicRunning) { // Need to do some cleanup
			// Remove all named entities
			for( std::map< std::string, long long >::iterator it = namedEntityMap.begin(); it != namedEntityMap.end(); ++it ) {
				app.RemoveEntity( it->second );
			}

			// Stop all dialogue
			app.PlayDialogue("silence.ogg");

			app.SetOption("ParticleType", "Binary");
		}
	}
}

void CinematicEngine::DrawCinematicView() {

	// Cinematic 'Bar Lines'
	sf::RectangleShape box( sf::Vector2f(app.GetSize().x, 140) );
	box.setFillColor( sf::Color(0, 0, 0, 255) );

	box.setPosition(0, 0);
	app.Draw(box);
	box.setPosition(0, app.GetSize().y-140);
	app.Draw(box);

	// Drawing the cinematic title for a few seconds
	sf::Text n( sf::String(std::string("\'") + longName + "\'"), app.FindFont("Temp7c.ttf", 21), 21 );
	n.setPosition( 30, 100 );

	n.setColor( sf::Color(255, 255, 255, 0) );

	if( masterClock.getElapsedTime().asSeconds() < 6.0f ) {
		int alpha = 255-abs(((3-masterClock.getElapsedTime().asSeconds())/3)*255);
		n.setColor( sf::Color(255, 255, 255, alpha) );
	}

	
	app.Draw(n);
}