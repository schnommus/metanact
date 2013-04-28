#include "DefinedEntity.h"

DefinedEntity::DefinedEntity( App2D &app, std::string type, int xpos, int ypos, sf::Vector2f sVel, float sRotation, bool iw, std::string dispName, float mscale ) : Entity(app) {
		
	Entity::type = type;
	hStepEvent = app.GetEventHandler().stepEvent.attach( this, &DefinedEntity::onStep );
	x = xpos;
	y = ypos;
	vel = sVel;
	rotation = sRotation;
	health = 0;
	inWarp = iw;
	warpId = 0;
	onUnlockOnly = false;
	cinematicEntity = false;
	displayName = dispName;
	scale = mscale;
	isPersistant = false;

	std::ifstream ifs;
	type = "../media/entity/" + type + ".edef";
	ifs.open(type.c_str(), std::ios::in);

	if( !ifs.is_open() ) {
		throw std::exception(std::string("Invalid entity definition name" +  type).c_str());
	}

	// Decode all tags in the file and apply them
		
	drawOverride = false;

	while( !ifs.eof() ) {
		std::string tag;
		ifs >> tag;
		if ( tag == "NAME" ) {
			ifs >> type;
		} else if ( tag == "FILE" ) {
			/*std::string fname;
			ifs >> fname;
			fname = "../media/entity/shape/" + fname + ".sdef";
			sv = LineReader::Read(fname.c_str());*/
		} else if ( tag == "IMAGE" ) {
			std::string iname;
			ifs >> iname;
			imageDir = iname;
			imageSprite.setTexture( app.FindTexture(iname) );
			imageSprite.setOrigin( imageSprite.getLocalBounds().width/2, imageSprite.getLocalBounds().height/2 );
		} else if (tag == "SCALE") {
			float scalev;
			ifs >> scalev;
			tagList.push_back( boost::shared_ptr<Tag>( new ScaleTag( *this, scalev ) ) );
		} else if (tag == "ROTATES") {
			float rotatev;
			ifs >> rotatev;
			tagList.push_back( boost::shared_ptr<Tag>( new RotatesTag( *this, rotatev ) ) );
		} else if (tag == "WOBBLES") {
			float wfv, wsv;
			ifs >> wfv >> wsv;
			tagList.push_back( boost::shared_ptr<Tag>( new WobblesTag( *this, wfv, wsv ) ) );
		} else if (tag == "CREATES_GRAVITY") {
			float grv, gsv;
			bool sr;
			ifs >> grv >> gsv >> sr;
			tagList.push_back( boost::shared_ptr<Tag>( new CreatesGravityTag( *this, grv, gsv, sr ) ) );
		}  else if (tag == "USE_RANDOM_ROTATION") {
			tagList.push_back( boost::shared_ptr<Tag>( new UseRandomRotationTag( *this ) ) );
		} else if (tag == "USE_RANDOM_POSITION") {
			tagList.push_back( boost::shared_ptr<Tag>( new UseRandomPositionTag( *this ) ) );
		} else if (tag == "DESTROY_TYPE_ON_RADIUS") {
			std::string t;
			float r;
			ifs >> t >> r;
			tagList.push_back( boost::shared_ptr<Tag>( new DestroyTypeOnRadiusTag( *this, t, r ) ) );
		} else if (tag == "OVERRIDE_DRAW") {
			drawOverride = true;
		} else if (tag == "DECAYS") {
			float sa, ss, fa, fs, da, ds;
			ifs >> sa >> ss >> fa >> fs >> da >> ds;
			tagList.push_back( boost::shared_ptr<Tag>( new DecaysTag( *this, sa, ss, fa, fs, da, ds ) ) );
		} else if (tag == "HAS_VELOCITY") {
			tagList.push_back( boost::shared_ptr<Tag>( new HasVelocityTag( *this ) ) );
		} else if (tag == "VELOCITY_INHERITANCE") {
			float vi;
			ifs >> vi;
			tagList.push_back( boost::shared_ptr<Tag>( new VelocityInheritanceTag( *this, vi ) ) );
		} else if (tag == "INITIAL_VELOCITY") {
			float iv;
			ifs >> iv;
			tagList.push_back( boost::shared_ptr<Tag>( new InitialVelocityTag( *this, iv ) ) );
		} else if (tag == "INHERITED_HEADING_VELOCITY") {
			float vi;
			ifs >> vi;
			tagList.push_back( boost::shared_ptr<Tag>( new InheritedHeadingVelocityTag( *this, vi ) ) );
		} else if (tag == "AFFECTED_BY_GRAVITY") {
			float va;
			ifs >> va;
			tagList.push_back( boost::shared_ptr<Tag>( new AffectedByGravityTag( *this, va ) ) );
		} else if (tag == "VELOCITY_DETERMINES_HEADING") {
			tagList.push_back( boost::shared_ptr<Tag>( new VelocityDeterminesHeadingTag( *this ) ) );
		} else if (tag == "PROJECTILE_ON_TYPE_RADIUS") {
			std::string pt, tt;
			float dr, r;
			ifs >> pt >> tt >> dr >> r;
			tagList.push_back( boost::shared_ptr<Tag>( new ProjectileOnTypeRadiusTag( *this, pt, tt, dr, r ) ) );
		} else if (tag == "WARP_TARGET") {
			int d; short wid;
			ifs >> d >> wid;
			tagList.push_back( boost::shared_ptr<Tag>( new WarpTargetTag( *this, d, wid ) ) );
		} else if (tag == "WARP_TYPE_ON_RADIUS") {
			std::string wt;
			float wr; short wid;
			ifs >> wt >> wr >> wid;
			tagList.push_back( boost::shared_ptr<Tag>( new WarpTypeOnRadiusTag( *this, wt, wr, wid ) ) );
		} else if (tag == "VELOCITY_CLAMP") {
			float mim, mam;
			ifs >> mim >> mam;
			tagList.push_back( boost::shared_ptr<Tag>( new VelocityClampTag( *this, mim, mam ) ) );
		} else if (tag == "IS_LOCAL_PLAYER") {
			tagList.push_back( boost::shared_ptr<Tag>( new IsLocalPlayerTag( *this ) ) );
		} else if (tag == "HAS_HEALTH") {
			int amt;
			ifs >> amt;
			tagList.push_back( boost::shared_ptr<Tag>( new HasHealthTag( *this, amt ) ) );
		} else if (tag == "HURT_TYPE_ON_RADIUS") {
			std::string tp;
			int at, r;
			ifs >> tp >> at >> r;
			tagList.push_back( boost::shared_ptr<Tag>( new HurtTypeOnRadiusTag( *this, tp, at, r ) ) );
		} else if (tag == "PROJECTILE_ON_DESTROY") {
			std::string tp; int s; float ch;
			ifs >> tp >> s >> ch;
			tagList.push_back( boost::shared_ptr<Tag>( new ProjectileOnDestroyTag( *this, tp, s, ch ) ) );
		} else if (tag == "INCREASE_SCORE_ITEM") {
			int amt, r, loot; 
			ifs >> amt >> r >> loot;
			tagList.push_back( boost::shared_ptr<Tag>( new IncreaseScoreItemTag( *this, amt, r, loot ) ) );
		} else if (tag == "SOUND_ON_CREATE") {
			std::string fn;
			ifs >> fn;
			tagList.push_back( boost::shared_ptr<Tag>( new SoundOnCreateTag( *this, fn ) ) );
		} else if (tag == "IS_ENEMY") {
			tagList.push_back( boost::shared_ptr<Tag>( new IsEnemyTag( *this ) ) );
		} else if (tag == "APPROACH_PLAYER") {
			int nv, fv; float spd;
			ifs >> nv >> fv >> spd;
			tagList.push_back( boost::shared_ptr<Tag>( new ApproachPlayerTag( *this, nv, fv, spd ) ) );
		} else if (tag == "SHOOT_AT_PLAYER") {
			int ct; float fr; std::string pt;
			ifs >> ct >> fr >> pt;
			tagList.push_back( boost::shared_ptr<Tag>( new ShootAtPlayerTag( *this, ct, fr, pt ) ) );
		} else if (tag == "REMEMBER_DESTRUCTION") {
			tagList.push_back( boost::shared_ptr<Tag>( new RememberDestructionTag( *this ) ) );
		} else if (tag == "EMIT_SMOKE") {
			tagList.push_back( boost::shared_ptr<Tag>( new EmitSmokeTag( *this ) ) );
		} else if (tag == "ON_UNLOCK") {
			onUnlockOnly = true;
		} else if (tag == "IS_CINEMATIC_ENTITY") {
			cinematicEntity = true;
		} else if (tag == "HAS_SHADER") {
			std::string sn;
			ifs >> sn;
			tagList.push_back( boost::shared_ptr<Tag>( new HasShaderTag( *this, sn ) ) );
		} else if (tag == "IS_PERSISTANT") {
			isPersistant = true;
		}
	}

	ifs.close();
		
	for( int i = 0; i != tagList.size(); ++i ) {
		tagList[i]->Init();
	}
}

bool DefinedEntity::onStep( float delta ) {
	if( (!onUnlockOnly || app.currentLevelUnlocked) ) {
		for( int i = 0; i != tagList.size(); ++i ) {
			if(app.inGame && isBeingDrawn) tagList[i]->Step(delta);
		}
	}
	return true;
}

void DefinedEntity::Draw() {
	int esize = 50;

	isBeingDrawn = true;

	if ( app.renderWindow.convertCoords( sf::Vector2i(0, 0) ).x-esize < x &&
		 app.renderWindow.convertCoords( sf::Vector2i(0, 0) ).y-esize < y &&
		 app.renderWindow.convertCoords( sf::Vector2i(app.GetSize().x, app.GetSize().y ) ).x+esize > x &&
		 app.renderWindow.convertCoords( sf::Vector2i(app.GetSize().x, app.GetSize().y ) ).y+esize > y &&
		 (!onUnlockOnly || app.currentLevelUnlocked) && (!app.cinematicEngine.IsCinematicRunning() || cinematicEntity) ) {
		if( !drawOverride ) {
			if( !imageDir.empty() ){
				imageSprite.setPosition(x, y);
				imageSprite.setRotation(rotation);
				imageSprite.setScale(scale, scale);
				app.Draw(imageSprite);
				if(isPersistant)
					app.persistanceTarget.draw(imageSprite);
			}
		}

		if( displayName.length() > 0 ) {
			// Set the alpha of the name depending on screen position
			sf::Vector2f camPos = app.renderWindow.convertCoords( sf::Vector2i(app.GetSize().x/2, app.GetSize().y/2) );
			int dist = sqrt(pow(x-camPos.x,2) + pow(y-camPos.y,2));
			int a = app.GetSize().x/3-dist;
			if( a < 0 ) a = 0; else if (a > 150) a = 150;

			// Just keep names drawn for now. Overrides above
			a=160;

			// Draw the entity display name
			sf::Text n( sf::String(displayName), app.FindFont("Temp7c.ttf"), 15 );
			int offs = n.getLocalBounds().width;
			n.setPosition( x - offs/2, y - 60 );
			n.setColor( sf::Color(255, 255, 255, a) );
			app.Draw(n);
		}

		if(!inWarp) {
			for( int i = 0; i != tagList.size(); ++i ) {
				tagList[i]->Draw();
			}
		}
	}
}

DefinedEntity::~DefinedEntity() {
	if( !app.isClosing ) { // Some of these create entities - which we don't want when the program is closing/changing level!
		for( int i = 0; i != tagList.size(); ++i ) {
			tagList[i]->Destroy();
		}
	}
	app.GetEventHandler().stepEvent.detach(hStepEvent);
}