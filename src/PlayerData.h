#pragma once

#include <string>
#include <map>
#include <memory>

#include "Entity.h"
#include "LootData.h"


class App2D;

class PlayerData {
public:
	PlayerData( App2D &a );
	void Init();
	void AttachLootToPlayer( Entity *player );

	std::vector<std::string> FoundLootTypes();

	Loot &GetLootOfType(std::string type);

	std::string GetLootTypeForName( std::string name );

	void SetCurrentWeapon( std::string filename );

	WeaponLoot &CurrentWeaponDetails();

private:
	void LoadLootTypes();
	void PopulateLootDetails();

	std::shared_ptr<Loot> LootFromFile( std::string file );

	App2D &app;

	std::string currentWeaponFile;
	
	std::vector< std::string > foundLootTypes;

	std::vector< std::string > lootTypes;

	//Map of filename (without extension) to loot details
	std::map < std::string, std::shared_ptr<Loot> > lootDetails;
};