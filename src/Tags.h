#ifndef TAGS_H
#define TAGS_H

#include "DefinedEntity.h"
#include <SFML\Audio.hpp>
#include "Tag.h"

// Various properties an entity can have

class ScaleTag : public Tag {
public:
	ScaleTag( Entity &entityReference, float scale );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float initialScaleFactor;
};

class RotatesTag : public Tag {
public:
	RotatesTag( Entity &entityReference, float rotationSpeed );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float rSpeed;
};

class WobblesTag : public Tag {
public:
	WobblesTag( Entity &entityReference, float wobbleFactor, float wobbleSpeed );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float wFactor, wSpeed, iScale, i;
};

class CreatesGravityTag : public Tag {
public:
	CreatesGravityTag( Entity &entityReference, float gravityRadius, float gravityStrength, bool showRadius );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float gRadius, gStrength;
	bool sRadius; short alt;
//	sf::Shape rCircle;
};

class UseRandomRotationTag : public Tag {
public:
	UseRandomRotationTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
};

class DestroyTypeOnRadiusTag : public Tag {
public:
	DestroyTypeOnRadiusTag( Entity &entityReference, std::string typeToDestroy, float destructionRadius );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	std::string type;
	float dRadius;
};

class ProjectileOnTypeRadiusTag : public Tag {
public:
	ProjectileOnTypeRadiusTag( Entity &entityReference, std::string projectileType,
		std::string targetType, float detectionRadius, float rate );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	std::string tType, pType;
	float dRadius, r;
	sf::Clock timer;
};

class UseRandomPositionTag : public Tag {
public:
	UseRandomPositionTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
};

class DecaysTag : public Tag {
public:
	DecaysTag( Entity &entityReference, float startingAlpha, float startingScale,
		float finalAlpha, float finalScale, float deltaAlpha, float deltaScale );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float sAlpha, fAlpha, sScale, fScale, dAlpha, dScale;
	bool scaleUp, alphaUp;
};

class HasVelocityTag : public Tag {
public:
	HasVelocityTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
};

class InitialVelocityTag : public Tag {
public:
	InitialVelocityTag( Entity &entityReference, float amount );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float myAmount;
};

class VelocityInheritanceTag : public Tag {
public:
	VelocityInheritanceTag( Entity &entityReference, float inheritanceFactor );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float f;
};

class InheritedHeadingVelocityTag : public Tag {
public:
	InheritedHeadingVelocityTag( Entity &entityReference, float inheritanceFactor );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float f;
};

class AffectedByGravityTag : public Tag {
public:
	AffectedByGravityTag( Entity &entityReference, float gravityAttraction );
	virtual void Init();
	virtual void Step(float delta);
	bool onGravity( boost::any data );
	virtual void Draw();
	virtual void Destroy();
	float gAttraction;
	CppEventHandler hGravityEvent;
};

class VelocityDeterminesHeadingTag : public Tag {
public:
	VelocityDeterminesHeadingTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
};

class WarpTargetTag : public Tag {
public:
	WarpTargetTag( Entity &entityReference, int diffusion, short warpId );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	int d; short wid;
};

class WarpTypeOnRadiusTag : public Tag {
public:
	WarpTypeOnRadiusTag( Entity &entityReference, std::string typeToWarp, float warpRadius, short wid );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	std::string type;
	float wRadius; short wid;
};

class VelocityClampTag : public Tag {
public:
	VelocityClampTag( Entity &entityReference, float min, float max );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	float min, max;
};

class IsLocalPlayerTag : public Tag {
public:
	IsLocalPlayerTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	sf::Vector2f cdelta;
	sf::Clock bClock, bClock2, sClock;
	float agility, fireRate, fireRate2;
	std::string projectileType, projectileType2;
	int lHealth, lScore;
};

class HasHealthTag : public Tag {
public:
	HasHealthTag( Entity &entityReference, int amount );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	int amt;
};

class HurtTypeOnRadiusTag : public Tag {
public:
	HurtTypeOnRadiusTag( Entity &entityReference, std::string typeToHurt, int amountToHurt, int radius );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	std::string tth;
	int ath, r;
};

class ProjectileOnDestroyTag : public Tag {
public:
	ProjectileOnDestroyTag ( Entity &entityReference, std::string typeOfProjectile, int spread, float chancev );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	std::string tp; int s; float chance;
};

class IncreaseScoreItemTag : public Tag {
public:
	IncreaseScoreItemTag ( Entity &entityReference, int amountToIncrease, int radius, bool loot);
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	int amt, r; std::string lName; bool isLoot;
};

class SoundOnCreateTag : public Tag {
public:
	SoundOnCreateTag ( Entity &entityReference, std::string fileName);
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	std::string fName;
};

class IsEnemyTag : public Tag {
public:
	IsEnemyTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
};

class RememberDestructionTag : public Tag {
public:
	RememberDestructionTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
};

class EmitSmokeTag : public Tag {
public:
	EmitSmokeTag( Entity &entityReference );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	sf::Clock sClock;
};

class ApproachPlayerTag : public Tag {
public:
	ApproachPlayerTag( Entity &entityReference, int nearval, int farval, float speedval );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	int nearv;
	int farv;
	float speed;
};

class ShootAtPlayerTag : public Tag {
public:
	ShootAtPlayerTag( Entity &entityReference, int cutoffv, float fireRatev, std::string projectileTypev  );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	int cutoff; float fireRate;
	std::string projectileType;
	sf::Clock bClock;
};

class HasShaderTag : public Tag {
public:
	HasShaderTag( Entity &entityReference, std::string shaderNamev );
	virtual void Init();
	virtual void Step(float delta);
	virtual void Draw();
	virtual void Destroy();
	std::string shaderName;
};

#endif
