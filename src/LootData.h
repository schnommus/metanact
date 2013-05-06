#pragma once

#include <string>

struct Loot {
	virtual ~Loot() { }

	std::string fileName;
	std::string realName;
	std::string description;
	std::string category;
};

struct WeaponLoot : public Loot {
	float fireRate, projectionDistance, numRailProjectiles;
	bool doesCharge;
	std::string projectileType;
};

struct AntiGravLoot : public Loot {
	float gravityFactor;
};