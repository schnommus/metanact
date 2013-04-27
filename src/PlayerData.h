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

	std::vector<std::string> AllLootTypes();

	Loot &GetLootOfType(std::string type);

	std::string GetLootTypeForName( std::string name );

	void AddFoundLoot( std::string name );

	void EquipDefaults();

	void SetCurrentWeapon( std::string filename );

	WeaponLoot &CurrentWeaponDetails();

	void SetCurrentAntiGrav( std::string filename );

	AntiGravLoot &CurrentAntiGravDetails();

private:
	void LoadLootTypes();
	void PopulateLootDetails();

	std::shared_ptr<Loot> LootFromFile( std::string file );

	App2D &app;

	std::string currentWeaponFile;
	std::string currentAntiGravFile;
	
	std::vector< std::string > foundLootTypes;

	std::vector< std::string > lootTypes;

	//Map of filename (without extension) to loot details
	std::map < std::string, std::shared_ptr<Loot> > lootDetails;
};