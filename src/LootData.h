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
	float fireRate;
	std::string projectileType;
};