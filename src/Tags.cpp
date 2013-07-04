#include "Tags.h"
#include <sstream>



ScaleTag::ScaleTag( Entity &entityReference, float scale ) : Tag( entityReference), initialScaleFactor(scale) { }
void ScaleTag::Init() { e.scale = initialScaleFactor; }
void ScaleTag::Step(float delta) {}
void ScaleTag::Draw() {}
void ScaleTag::Destroy() {}

RotatesTag::RotatesTag( Entity &entityReference, float rotationSpeed ) : Tag( entityReference), rSpeed(rotationSpeed) { }
void RotatesTag::Init() {}
void RotatesTag::Step(float delta) { e.rotation += rSpeed * delta; }
void RotatesTag::Draw() {}
void RotatesTag::Destroy() {}



WobblesTag::WobblesTag( Entity &entityReference, float wobbleFactor, float wobbleSpeed ) 
	: Tag( entityReference), wFactor(wobbleFactor), wSpeed(wobbleSpeed), i((rand()%360)/180.0*3.14) { }
void WobblesTag::Init() { iScale = e.scale; }

void WobblesTag::Step(float delta) {
	i += wSpeed * delta;
	e.scale = iScale + wFactor*sin(i);
}

void WobblesTag::Draw() {}
void WobblesTag::Destroy() {}


CreatesGravityTag::CreatesGravityTag( Entity &entityReference, float gravityRadius, float gravityStrength, bool showRadius ) 
	: Tag( entityReference), gRadius(gravityRadius), gStrength(gravityStrength), sRadius(showRadius) { }
	
void CreatesGravityTag::Init() { alt = e.id%3; }

void CreatesGravityTag::Step(float delta) {
	//if(sRadius)
//		rCircle = sf::Shape::Circle(e.x, e.y, gRadius, sf::Color(0, 0, 0, 0), 2, sf::Color( 20, 0, 0 ) );
	
	// This makes gravity calculation stagger between frames, reducing CPU load
	++alt;
	if( alt%3==0 )
		e.app.GetEventHandler().NotifyGenericEvent("gravity", GravityData(e.x, e.y, gRadius, gStrength*3, delta));
}

void CreatesGravityTag::Draw() {
	//if(sRadius)
		//e.app.Draw( rCircle );
}

void CreatesGravityTag::Destroy() {}




UseRandomRotationTag::UseRandomRotationTag( Entity &entityReference ) : Tag( entityReference) { }
void UseRandomRotationTag::Init() { e.rotation = rand()%360; }
void UseRandomRotationTag::Step(float delta) {}
void UseRandomRotationTag::Draw() {}
void UseRandomRotationTag::Destroy() {}




DestroyTypeOnRadiusTag::DestroyTypeOnRadiusTag( Entity &entityReference, std::string typeToDestroy, float destructionRadius )
	: Tag( entityReference), type(typeToDestroy),  dRadius(destructionRadius) { }
void DestroyTypeOnRadiusTag::Init() { }

void DestroyTypeOnRadiusTag::Step(float delta) {
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
			it != e.app.entities.end();
			++it) {
		if( it->second->type == type && !it->second->inWarp) {
			float dist = sqrt(powf(e.x-it->second->x, 2) + powf(e.y-it->second->y, 2));
			if( dist < dRadius )
				e.app.RemoveEntity( it->second->id );
		}
	}
}

void DestroyTypeOnRadiusTag::Draw() {}
void DestroyTypeOnRadiusTag::Destroy() {}



ProjectileOnTypeRadiusTag::ProjectileOnTypeRadiusTag( Entity &entityReference, std::string projectileType, std::string targetType, float detectionRadius, float rate )
	: Tag( entityReference), tType(targetType), pType(projectileType), dRadius(detectionRadius), r(rate) { }
void ProjectileOnTypeRadiusTag::Init() { timer.restart(); }

void ProjectileOnTypeRadiusTag::Step(float delta) {
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
			it != e.app.entities.end();
			++it) {
		if( it->second->type == tType && timer.getElapsedTime().asSeconds() > 1.0/r ) {
			timer.restart();
			float dist = sqrt(powf(e.x-it->second->x, 2) + powf(e.y-it->second->y, 2));
			if( dist < dRadius )
				e.app.AddEntity( new DefinedEntity(e.app, pType, e.x, e.y, e.vel, e.rotation ), 10 );
		}
	}
}

void ProjectileOnTypeRadiusTag::Draw() {}
void ProjectileOnTypeRadiusTag::Destroy() {}




UseRandomPositionTag::UseRandomPositionTag( Entity &entityReference ) : Tag( entityReference) { }

void UseRandomPositionTag::Init() {
	int widthx = int(e.app.field.tright.x-e.app.field.bleft.x);
	int widthy = int(e.app.field.tright.y-e.app.field.bleft.y);
	e.x = (rand() % widthx)-widthx/2;
	e.y = (rand() % widthy)-widthy/2;
}

void UseRandomPositionTag::Step(float delta) {}
void UseRandomPositionTag::Draw() {}
void UseRandomPositionTag::Destroy() {}



DecaysTag::DecaysTag( Entity &entityReference, float startingAlpha, float startingScale,
	float finalAlpha, float finalScale, float deltaAlpha, float deltaScale )
	: Tag( entityReference), sAlpha(startingAlpha), fAlpha(finalAlpha),
	sScale(startingScale), fScale(finalScale), dAlpha(deltaAlpha), dScale(deltaScale) {
		scaleUp = alphaUp = false;
		if( sScale < fScale ) scaleUp = true;
		if( sAlpha < fAlpha ) alphaUp = true;
}

void DecaysTag::Init() {}

void DecaysTag::Step(float delta) {
	e.displayName = "";
	sAlpha -= dAlpha*delta;
	sScale -= dScale*delta;
	if( !alphaUp ) { if( sAlpha < fAlpha ) { e.app.RemoveEntity( e.id ); } }
	else { if( sAlpha >= fAlpha ) { e.app.RemoveEntity( e.id ); } }
	if( !scaleUp ) { if( sScale < fScale ) { e.app.RemoveEntity( e.id ); } }
	else { if( sScale >= fScale ) { e.app.RemoveEntity( e.id ); } }
}

void DecaysTag::Draw() {
/*	for( int i = 0; i != e.sv.size(); ++i ) {
		sf::Color c = e.sv[i].GetColor();
		c.a = sAlpha;
		e.sv[i].SetPosition(e.x, e.y);
		e.sv[i].SetRotation(e.rotation);
		e.sv[i].SetScale(sScale, sScale);
		e.sv[i].SetColor( c );
		e.app.Draw(e.sv[i]);
	}*/
	e.imageSprite.setPosition(e.x, e.y);
	e.imageSprite.setColor(sf::Color(255, 255, 255, sAlpha));
	e.imageSprite.setRotation(e.rotation);
	e.imageSprite.setScale(e.scale, e.scale);
	e.app.Draw(e.imageSprite);
	if(e.isPersistant)
		e.app.persistanceTarget.draw(e.imageSprite);
}

void DecaysTag::Destroy() {}



HasVelocityTag::HasVelocityTag( Entity &entityReference ) : Tag( entityReference) { }
void HasVelocityTag::Init() {}

void HasVelocityTag::Step(float delta) {
	e.x += 10*e.vel.x*delta;
	e.y += 10*e.vel.y*delta;
	std::string snd("ticksound.wav");
	//Anything with a velocity must be kept on the field, play tick sound with hits (doesn't apply to cinematic ent's
	if( !e.cinematicEntity ) {
		if( e.x < e.app.field.bleft.x ) { e.vel.x = -e.vel.x*0.5; e.x = e.app.field.bleft.x+2; e.app.PlaySound(snd, false, true, e.x, e.y);}
		if( e.y < e.app.field.bleft.y ) { e.vel.y = -e.vel.y*0.5; e.y = e.app.field.bleft.y+2; e.app.PlaySound(snd, false, true, e.x, e.y);}
		if( e.x > e.app.field.tright.x ) { e.vel.x = -e.vel.x*0.5; e.x = e.app.field.tright.x-2; e.app.PlaySound(snd, false, true, e.x, e.y);}
		if( e.y > e.app.field.tright.y ) { e.vel.y = -e.vel.y*0.5; e.y = e.app.field.tright.y-2; e.app.PlaySound(snd, false, true, e.x, e.y);}
	}
}

void HasVelocityTag::Draw() {}
void HasVelocityTag::Destroy() {}



InitialVelocityTag::InitialVelocityTag( Entity &entityReference, float amount ) : Tag( entityReference), myAmount(amount) { }
void InitialVelocityTag::Init() { 
	e.vel.x += myAmount*sin((e.rotation+180)/180*3.14);
	e.vel.y += myAmount*cos((e.rotation+180)/180*3.14);
}
void InitialVelocityTag::Step(float delta) {}
void InitialVelocityTag::Draw() {}
void InitialVelocityTag::Destroy() {}




VelocityInheritanceTag::VelocityInheritanceTag( Entity &entityReference, float inheritanceFactor )
	: Tag( entityReference ), f(inheritanceFactor) { }
void VelocityInheritanceTag::Init() { e.vel.x *= f; e.vel.y *= f; }
void VelocityInheritanceTag::Step(float delta) {}
void VelocityInheritanceTag::Draw() {}
void VelocityInheritanceTag::Destroy() {}



InheritedHeadingVelocityTag::InheritedHeadingVelocityTag( Entity &entityReference, float inheritanceFactor )
	: Tag( entityReference ), f(inheritanceFactor) { }

void InheritedHeadingVelocityTag::Init() { 
	e.vel.x -= f*sinf( -e.rotation/180*3.14 );
	e.vel.y -= f*cosf( -e.rotation/180*3.14 );
}

void InheritedHeadingVelocityTag::Step(float delta) {}
void InheritedHeadingVelocityTag::Draw() {}
void InheritedHeadingVelocityTag::Destroy() {}




AffectedByGravityTag::AffectedByGravityTag( Entity &entityReference, float gravityAttraction)
	: Tag( entityReference ), gAttraction(gravityAttraction) { }
	
void AffectedByGravityTag::Init() { 
	hGravityEvent = e.app.GetEventHandler().GetGenericEvent("gravity").attach( this, &AffectedByGravityTag::onGravity );
}

void AffectedByGravityTag::Step(float delta) { }

bool AffectedByGravityTag::onGravity( boost::any data ) {
	GravityData d = boost::any_cast<GravityData>(data);
	float dist = sqrt(powf(e.x-d.x, 2) + powf(e.y-d.y, 2));
	if( dist < d.radius && dist > 3) {
		e.vel.x += (d.x-e.x)/pow(dist, 2)*d.strength*gAttraction*d.delta;
		e.vel.y += (d.y-e.y)/pow(dist, 2)*d.strength*gAttraction*d.delta;
	}
	return true;
}

void AffectedByGravityTag::Draw() {}

void AffectedByGravityTag::Destroy() {
	e.app.GetEventHandler().GetGenericEvent("gravity").detach(hGravityEvent);
}




VelocityDeterminesHeadingTag::VelocityDeterminesHeadingTag( Entity &entityReference ) : Tag( entityReference) { }
 void VelocityDeterminesHeadingTag::Init() {}

 void VelocityDeterminesHeadingTag::Step(float delta) {
	e.rotation = -(180+atan2f(e.vel.x, e.vel.y) / 3.14 * 180);
}

 void VelocityDeterminesHeadingTag::Draw() {}
 void VelocityDeterminesHeadingTag::Destroy() {}



WarpTargetTag::WarpTargetTag( Entity &entityReference, int diffusion, short warpId ) : Tag( entityReference), d(diffusion), wid(warpId) { }
void WarpTargetTag::Init() { }

void WarpTargetTag::Step(float delta) {
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
			it != e.app.entities.end();
			++it) {
		if( it->second->inWarp && it->second->warpId == wid && it->second->warpParent != e.id) {
			if( rand()%5 == 0 ) { //Random target selection
				it->second->x = e.x+rand()%(2*d)-d;
				it->second->y = e.y+rand()%(2*d)-d;
				it->second->inWarp = false;
			}
		}
	}
}

void WarpTargetTag::Draw() {}
void WarpTargetTag::Destroy() {}



WarpTypeOnRadiusTag::WarpTypeOnRadiusTag( Entity &entityReference, std::string typeToWarp, float warpRadius, short warpId )
	: Tag( entityReference), type(typeToWarp),  wRadius(warpRadius), wid(warpId) { }
void WarpTypeOnRadiusTag::Init() { if(e.displayName == "..") e.imageSprite.setColor(sf::Color(255, 255, 0)); }

void WarpTypeOnRadiusTag::Step(float delta) {
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
			it != e.app.entities.end();
			++it) {
		if( it->second->type == type && !it->second->inWarp  ) {
			float dist = sqrt(powf(e.x-it->second->x, 2) + powf(e.y-it->second->y, 2));
			if( dist < wRadius ) {
				if( it->second->type == "localplayer" ) {

					sf::Text n( sf::String("Loading..."), e.app.FindFont("Temp7c.ttf"), 40 );
					n.setPosition( e.x-60, e.y );
					n.setColor( sf::Color(255, 255, 255, 200) );
					e.app.Draw(n);


					e.app.currentPath = e.app.currentPath + "\\" + e.displayName;
				}
				it->second->inWarp = true;
				it->second->warpId = wid;
				it->second->warpParent = e.id;
				e.app.PlaySound("warpsound.wav", false, true, e.x, e.y);
			}
		}
	}
}

void WarpTypeOnRadiusTag::Draw() {}
void WarpTypeOnRadiusTag::Destroy() {}



inline void clamp( float &v, float a, float b ) {
	v = (v < a ? a : ( v > b ? b : v));
}

VelocityClampTag::VelocityClampTag( Entity &entityReference, float minv, float maxv )
		: Tag(entityReference), min(minv), max(maxv) { }

void VelocityClampTag::Init() {}
void VelocityClampTag::Step(float delta) {
	clamp( e.vel.x, min, max );
	clamp( e.vel.y, min, max );
}
void VelocityClampTag::Draw() {}
void VelocityClampTag::Destroy() {}



IsLocalPlayerTag::IsLocalPlayerTag( Entity &entityReference ) : Tag( entityReference) { }

void IsLocalPlayerTag::Init() { 
	e.app.PlayDialogue("okay.ogg");
	e.app.FollowEntity(e);
	agility = 50;
	fireRate = 5; projectileType = "b";
	fireRate2 = 0.25; projectileType2 = "mb";
	lHealth = e.health;
	fullyCharged = false;
}

void IsLocalPlayerTag::Step(float delta) {

	sf::Vector2f MousePos = e.app.renderWindow.convertCoords( sf::Vector2i(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y) );
	e.rotation = -(180+atan2f(MousePos.x-e.x, MousePos.y-e.y) / 3.14 * 180);

	if( e.app.GetOption("ControlScheme") == "Mouse Relative" ) {

		if( sf::Keyboard::isKeyPressed(sf::Keyboard::W) ) {
			e.vel.x += agility*delta*sin((e.rotation+180)/180*3.14);
			e.vel.y += agility*delta*cos((e.rotation+180)/180*3.14);
		}
		if( sf::Keyboard::isKeyPressed(sf::Keyboard::S) ) {
			e.vel.x += agility*delta*sin((e.rotation)/180*3.14);
			e.vel.y += agility*delta*cos((e.rotation)/180*3.14);
		}
		if( sf::Keyboard::isKeyPressed(sf::Keyboard::A) ) {
			e.vel.x += agility*delta*sin((e.rotation-90)/180*3.14);
			e.vel.y += agility*delta*cos((e.rotation-90)/180*3.14);
		}
		if( sf::Keyboard::isKeyPressed(sf::Keyboard::D) ) {
			e.vel.x += agility*delta*sin((e.rotation+90)/180*3.14);
			e.vel.y += agility*delta*cos((e.rotation+90)/180*3.14);
		}

	} else {

	//Original control scheme

		if( sf::Keyboard::isKeyPressed(sf::Keyboard::W) ) {
			e.vel.y -= agility*delta;
		}
		if( sf::Keyboard::isKeyPressed(sf::Keyboard::S) ) {
			e.vel.y += agility*delta;
		}
		if( sf::Keyboard::isKeyPressed(sf::Keyboard::A) ) {
			e.vel.x -= agility*delta;
		}
		if( sf::Keyboard::isKeyPressed(sf::Keyboard::D) ) {
			e.vel.x += agility*delta;
		}

	}

	projectileType = e.app.playerData.CurrentWeaponDetails().projectileType;
	fireRate = e.app.playerData.CurrentWeaponDetails().fireRate;

	float projectionDistance = e.app.playerData.CurrentWeaponDetails().projectionDistance;
	int numRailProjectiles = e.app.playerData.CurrentWeaponDetails().numRailProjectiles;

	bool doesCharge = e.app.playerData.CurrentWeaponDetails().doesCharge;

	if( doesCharge && !sf::Mouse::isButtonPressed( sf::Mouse::Left ) ) {
		if( bClock.getElapsedTime().asSeconds() > 1.0/fireRate ) {
			for( int i = 1; i <= numRailProjectiles; ++ i) {
				e.app.AddEntity( new DefinedEntity( e.app, projectileType,
					e.x + i*projectionDistance*sin((-e.rotation+180)/180*3.14),
					e.y + i*projectionDistance*cos((-e.rotation+180)/180*3.14),
					e.vel, e.rotation ), 10 );
			}
		}
		fullyCharged = false;
		bClock.restart();
	}

	if( doesCharge ) {
		e.scale = 1+(bClock.getElapsedTime().asSeconds() / (1.0/fireRate))*0.2;
		if( e.scale > 1.2 ) e.scale = 1.2;

		if( sf::Mouse::isButtonPressed( sf::Mouse::Left ) ) {
			if( bClock.getElapsedTime().asSeconds() < delta*2 ) {
				e.app.PlaySound("charging.ogg");
			}
			if( !fullyCharged && bClock.getElapsedTime().asSeconds() > 1.0/fireRate ) {
				fullyCharged = true;
				e.app.PlaySound("charged.ogg");
			}
		}
	}

	if( !doesCharge && sf::Mouse::isButtonPressed( sf::Mouse::Left ) && bClock.getElapsedTime().asSeconds() > 1.0/fireRate ) {
		for( int i = 1; i <= numRailProjectiles; ++ i) {
			e.app.AddEntity( new DefinedEntity( e.app, projectileType,
						e.x + i*projectionDistance*sin((-e.rotation+180)/180*3.14),
						e.y + i*projectionDistance*cos((-e.rotation+180)/180*3.14),
						e.vel, e.rotation ), 10 );
		}
		bClock.restart();
	}


	if( sClock.getElapsedTime().asSeconds() > 0.1 * e.app.EvaluateOption("ParticleDensity") ) {
		float spd = sqrt( pow(e.vel.x, 2) + pow(e.vel.y, 2) );
		e.app.AddEntity( new SmokeParticle(e.app, e.x, e.y, spd*1.5), 10, false );
		sClock.restart();
	}

	// Do antigrav stuff
	e.app.GetEventHandler().NotifyGenericEvent("gravity", GravityData(e.x, e.y, 170, e.app.playerData.CurrentAntiGravDetails().gravityFactor, delta));

	e.app.playerDeathTimer.restart();

	if( lHealth > e.health ) {
		e.app.PlaySound("hurtsound.wav");
	}

	lHealth = e.health;
}

void IsLocalPlayerTag::Draw() {}
void IsLocalPlayerTag::Destroy() { 
	e.app.StopFollowing();
	std::string deathSayings[6] = {
		" seems to have exploded",
		" had an untimely death",
		" now resembles a pincushion",
		" is no more",
		"'s ship imploded",
		" finds it hard to stay alive"
	};
	e.app.DisplayMessage( e.app.playerName + deathSayings[rand()%6] );
	e.app.DisplayBigMessage( e.app.playerName + deathSayings[rand()%6] );
	e.app.PlayDialogue("deathvocals.ogg");
	// Aaand launch an opening cinematic
	//e.app.cinematicEngine.RunCinematic("cin1.cnm");
}



HasHealthTag::HasHealthTag( Entity &entityReference, int amount )
 : Tag(entityReference), amt(amount) { }

void HasHealthTag::Init() { e.health = amt; }
void HasHealthTag::Step(float delta) {
	if( e.health <= 0 ) {
		e.app.RemoveEntity(e.id);
		e.health = 0;
	}
}

void HasHealthTag::Draw() {
	sf::RectangleShape r1( sf::Vector2f(amt, 8) );
	r1.setFillColor( sf::Color(180, 0, 0) );
	r1.setPosition(e.x-amt/2, e.y+65);
	e.app.Draw(r1);
	r1.setFillColor( sf::Color(0, 180, 0) );
	r1.setSize( sf::Vector2f( float(e.health), 8.0 ) );
	e.app.Draw(r1);
}

void HasHealthTag::Destroy() {}




HurtTypeOnRadiusTag::HurtTypeOnRadiusTag( Entity &entityReference, std::string typeToHurt, int amountToHurt, int radius )
	: Tag(entityReference), tth(typeToHurt), ath(amountToHurt), r(radius) { }
void HurtTypeOnRadiusTag::Init() { }
void HurtTypeOnRadiusTag::Step(float delta) {
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
			it != e.app.entities.end();
			++it) {
		if( it->second->type == tth ) {
			float dist = sqrt(powf(e.x-it->second->x, 2) + powf(e.y-it->second->y, 2));
			if( dist < r  && !it->second->inWarp) {
				it->second->health -= ath;
				e.app.RemoveEntity(e.id);
			}
		}
	}
}
void HurtTypeOnRadiusTag::Draw() { }
void HurtTypeOnRadiusTag::Destroy() { }


ProjectileOnDestroyTag::ProjectileOnDestroyTag ( Entity &entityReference, std::string typeOfProjectile, int spread, float chancev )
	: Tag(entityReference), tp(typeOfProjectile), s(spread), chance(chancev) { }
void ProjectileOnDestroyTag::Init() {}
void ProjectileOnDestroyTag::Step(float delta) {}
void ProjectileOnDestroyTag::Draw() {}
void ProjectileOnDestroyTag::Destroy() {
	if( s==0 ) s = 1; // Avoid division by zero for range
	// Avoid creation on level change
	if( !e.app.LevelChanged() && (float(rand()%10000)/10000.0) <= chance ) e.app.AddEntity( new DefinedEntity( e.app, tp, e.x+((rand()%s)*2)-s, e.y+((rand()%s)*2)-s, e.vel, e.rotation, false, e.displayName ), 10 );
}




IncreaseScoreItemTag::IncreaseScoreItemTag ( Entity &entityReference, int amountToIncrease, int radius, bool loot)
	: Tag(entityReference), amt(amountToIncrease), r(radius), isLoot(loot) { }

void IncreaseScoreItemTag::Init() {
	if( isLoot ) {
		std::vector<std::string> lTypes = e.app.playerData.AllLootTypes();
		std::vector<std::string> fTypes = e.app.playerData.FoundLootTypes();
		int index = 0;
		for( int i = 0; i <= 100; ++i ) {
			index = rand()%lTypes.size();
			if( std::find( fTypes.begin(), fTypes.end(), lTypes[index] ) == fTypes.end() ) {
				break;
			}
		}
		lName = lTypes[index];
		e.displayName = e.app.playerData.GetLootOfType( lName ).realName;
	} else {
		e.displayName = "Remnants of " + e.displayName;
	}
}
void IncreaseScoreItemTag::Step(float delta) {
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
			it != e.app.entities.end();
			++it) {
		if( it->second->type == "localplayer" ) {
			float dist = sqrt(powf(e.x-it->second->x, 2) + powf(e.y-it->second->y, 2));
			if( dist < r ) {
				std::string scoreSayings[4] = {
					"Found the ",
					"Obtained the ",
					"Recovered the ",
					"Discovered the "
				};

				std::ostringstream oss;

				if( isLoot ) {
					/*oss << e.app.playerName
						<< scoreSayings[rand()%4] << ": " << e.app.playerData.GetLootOfType( lName ).realName;*/
					oss << scoreSayings[rand()%4] << e.app.playerData.GetLootOfType( lName ).realName;
					e.app.playerData.AddFoundLoot(lName);

					e.app.DisplayMessage( oss.str() );
					e.app.PlaySound("loot.wav");

					//e.app.cinematicEngine.RunCinematic("cin1.cnm");

					e.app.DisplayBigMessage( oss.str() );
				} else {
					oss << "+10,000 qB";
					e.app.currentPlayerScore += 10000;
					e.app.currentPlayerScoreScale = 45;
					e.app.PlaySound("junksound.wav");


				}

				e.app.RemoveEntity(e.id);
			}
		}
	}
}
void IncreaseScoreItemTag::Draw() {}
void IncreaseScoreItemTag::Destroy() {}




SoundOnCreateTag::SoundOnCreateTag ( Entity &entityReference, std::string fileName) : Tag(entityReference), fName(fileName) { }

void SoundOnCreateTag::Init(){
	e.app.PlaySound(fName, false, true, e.x, e.y);
}

void SoundOnCreateTag::Step(float delta){}
void SoundOnCreateTag::Draw(){}
void SoundOnCreateTag::Destroy(){}



IsEnemyTag::IsEnemyTag( Entity &entityReference ) : Tag( entityReference) { }

void IsEnemyTag::Init() { 
	e.isEnemy = true;
}

void IsEnemyTag::Step(float delta) { }
void IsEnemyTag::Draw() {}
void IsEnemyTag::Destroy() {}



RememberDestructionTag::RememberDestructionTag( Entity &entityReference ) : Tag( entityReference) { }
void RememberDestructionTag::Init() {}
void RememberDestructionTag::Step(float delta) { }
void RememberDestructionTag::Draw() {}

void RememberDestructionTag::Destroy() {
	// Only if level isn't changing
	if( !e.app.LevelChanged() ) e.app.ReleaseFile(e.displayName, "destroyed");
}



EmitSmokeTag::EmitSmokeTag( Entity &entityReference ) : Tag( entityReference) { }
void EmitSmokeTag::Init() {}
void EmitSmokeTag::Step(float delta) { 
	if( sClock.getElapsedTime().asSeconds() > 0.3 * e.app.EvaluateOption("ParticleDensity") ) {
		float spd = sqrt( pow(e.vel.x, 2) + pow(e.vel.y, 2) );
		if(spd > 0.1) e.app.AddEntity( new SmokeParticle(e.app, e.x, e.y, spd*1.5), 10, false );
		sClock.restart();
	} 
}

void EmitSmokeTag::Draw() { }

void EmitSmokeTag::Destroy() { }



ApproachPlayerTag::ApproachPlayerTag( Entity &entityReference, int nearval, int farval, float speedval ) : Tag( entityReference), nearv(nearval), farv(farval), speed(speedval) { }
void ApproachPlayerTag::Init() {}

void ApproachPlayerTag::Step(float delta) {
	float px=0, py=0;
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
		it != e.app.entities.end();
		++it) {
			if( it->second->type == "localplayer" ) {
				px = it->second->x;
				py = it->second->y;
				float dist = sqrt(powf(e.x-it->second->x, 2) + powf(e.y-it->second->y, 2));
				if( dist < nearv ) {
					e.rotation = -(180+atan2f(px-e.x, py-e.y) / 3.14 * 180);
					e.vel.y -= speed*delta*cos((-e.rotation-180)/180*3.14);
					e.vel.x -= speed*delta*sin((-e.rotation-180)/180*3.14);
				} else if( dist < farv ) {
					e.rotation = -(180+atan2f(px-e.x, py-e.y) / 3.14 * 180);
					e.vel.y += speed*delta*cos((-e.rotation-180)/180*3.14);
					e.vel.x += speed*delta*sin((-e.rotation-180)/180*3.14);
				}
			}
	}
}

void ApproachPlayerTag::Draw() { }
void ApproachPlayerTag::Destroy() { }



ShootAtPlayerTag::ShootAtPlayerTag( Entity &entityReference, int cutoffv, float fireRatev, std::string projectileTypev ) : Tag( entityReference), cutoff(cutoffv), fireRate(fireRatev), projectileType(projectileTypev) { }
void ShootAtPlayerTag::Init() {}
void ShootAtPlayerTag::Step(float delta) {
	for( App2D::EntityMap::iterator it = e.app.entities.begin();
		it != e.app.entities.end();
		++it) {
			if( it->second->type == "localplayer" ) {
				float dist = sqrt(powf(e.x-it->second->x, 2) + powf(e.y-it->second->y, 2));
				if( dist < 600 ) {
					if( bClock.getElapsedTime().asSeconds() > 1.0/fireRate ) {
						e.app.AddEntity( new DefinedEntity( e.app, projectileType,
							e.x + 40*sin((-e.rotation+180)/180*3.14),
							e.y + 40*cos((-e.rotation+180)/180*3.14),
							e.vel, e.rotation ), 10 );
						bClock.restart();
					}
				}
			}
	}
}
void ShootAtPlayerTag::Draw() {}
void ShootAtPlayerTag::Destroy() {}


HasShaderTag::HasShaderTag( Entity &entityReference, std::string shaderNamev ) : Tag(entityReference), shaderName(shaderNamev) { }
void HasShaderTag::Init() {}
void HasShaderTag::Step(float delta) {}
void HasShaderTag::Draw() {
	int esize = 50;
	if( e.app.GetOption("UseShaders") == "Enabled" &&
		e.app.renderWindow.convertCoords( sf::Vector2i(0, 0) ).x-esize < e.x &&
		e.app.renderWindow.convertCoords( sf::Vector2i(0, 0) ).y-esize < e.y &&
		e.app.renderWindow.convertCoords( sf::Vector2i(e.app.GetSize().x, e.app.GetSize().y ) ).x+esize > e.x &&
		e.app.renderWindow.convertCoords( sf::Vector2i(e.app.GetSize().x, e.app.GetSize().y ) ).y+esize > e.y &&
		(!e.onUnlockOnly || e.app.currentLevelUnlocked) && (!e.app.cinematicEngine.IsCinematicRunning() || e.cinematicEntity) ) {
			sf::Vector2f bLeft = e.app.renderWindow.convertCoords( sf::Vector2i(0, 0) );
			sf::Vector2f tRight = e.app.renderWindow.convertCoords( sf::Vector2i(e.app.GetSize().x, e.app.GetSize().y) );

			sf::Vector2f v( (e.x - bLeft.x)/e.app.GetSize().x, 1.0-(e.y - bLeft.y)/e.app.GetSize().y );

			e.app.postProcessingStack.push( std::pair< std::string, sf::Vector2f >(shaderName, v) );
	}
}
void HasShaderTag::Destroy() {}


EmplaceEntityTag::EmplaceEntityTag( Entity &entityReference, float x, float y, std::string type ) : Tag(entityReference), xoffset(x), yoffset(y), theType(type) { }
void EmplaceEntityTag::Init() {
	childEntity = e.app.AddEntity( new DefinedEntity(e.app, theType), 12 );
}
void EmplaceEntityTag::Step(float delta) {
	Entity *childpt = e.app.GetEntityWithId(childEntity);
	childpt->x = e.x+xoffset;
	childpt->y = e.y+yoffset;
}

void EmplaceEntityTag::Draw() {}
void EmplaceEntityTag::Destroy() {
	e.app.RemoveEntity(childEntity);
}
