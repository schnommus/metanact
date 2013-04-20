#include "PlayerData.h"
#include <fstream>
#include <iostream>
#include <sstream>


PlayerData::PlayerData( App2D &a ) : app(a) { }

WeaponLoot &PlayerData::CurrentWeaponDetails() {
	return *dynamic_cast<WeaponLoot*>(lootDetails[currentWeaponFile].get());
}

void PlayerData::SetCurrentWeapon( std::string filename ) {
	currentWeaponFile = filename;
}

std::vector<std::string> PlayerData::FoundLootTypes() {
	return foundLootTypes;
}

Loot &PlayerData::GetLootOfType(std::string type) {
	return *lootDetails[type].get();
}

std::string PlayerData::GetLootTypeForName( std::string name ) {
	for( int i = 0; i != lootTypes.size(); ++i ) {
		if( lootDetails[ lootTypes[i] ]->realName ==  name ) {
			return lootTypes[i];
		}
	}
	return "";
}

void PlayerData::Init() {
	LoadLootTypes();
	PopulateLootDetails();

	// Equip the first loot entry as first weapon
	currentWeaponFile = lootTypes[0];

	// Just assume we've found everything for now
	foundLootTypes = lootTypes;
}

void PlayerData::LoadLootTypes() {
	std::ifstream ifs;
	ifs.open("../media/entity/!loot.ldb", std::ios::in);

	if( !ifs.is_open() ) {
		throw std::exception("Couldn't open loot database!");
	}

	while( !ifs.eof() ) {
		std::string all; getline(ifs, all);
		lootTypes.push_back( all );
	}

	ifs.close();
}

void PlayerData::PopulateLootDetails() {
	for( int i = 0; i != lootTypes.size(); ++i ) {
		lootDetails[ lootTypes[i] ] = LootFromFile(lootTypes[i]);
	}
}

std::shared_ptr<Loot> PlayerData::LootFromFile( std::string file ) {
	std::ifstream ifs;
	ifs.open("../media/entity/loot/" + file, std::ios::in);

	if( !ifs.is_open() ) {
		throw std::exception(std::string("Couldn't open loot file: " + file).c_str());
	}

	std::string category; getline(ifs, category);

	Loot *theLoot = nullptr;
	
	// Files are currently order-dependant
	// Thinking of moving everything over to XML

	if( category == "WEAPON" ) {
		WeaponLoot *weaponLoot = new WeaponLoot;

		weaponLoot->fileName = file; //filename

		weaponLoot->category = category; //category

		std::string line; getline(ifs, line); // name
		weaponLoot->realName = line;

		getline(ifs, line); // firerate
		std::istringstream iss(line);
		iss >> weaponLoot->fireRate;

		getline(ifs, line); // projectile type
		weaponLoot->projectileType = line;

		getline(ifs, line); // description
		weaponLoot->description = line;
		theLoot = weaponLoot;

		std::cout << "Loot type loaded: " << weaponLoot->realName << std::endl;
	}

	ifs.close();

	return std::shared_ptr<Loot>(theLoot);
}