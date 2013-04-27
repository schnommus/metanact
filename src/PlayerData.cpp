#include "PlayerData.h"
#include "App2D.h"

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

AntiGravLoot &PlayerData::CurrentAntiGravDetails() {
	return *dynamic_cast<AntiGravLoot*>(lootDetails[currentAntiGravFile].get());
}

void PlayerData::SetCurrentAntiGrav( std::string filename ) {
	currentAntiGravFile = filename;
}

std::vector<std::string> PlayerData::FoundLootTypes() {
	return foundLootTypes;
}

std::vector<std::string> PlayerData::AllLootTypes() {
	return lootTypes;
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

	// Equip some default loot
	EquipDefaults();

	// To assume we've found everything
	// foundLootTypes = lootTypes;
}

void PlayerData::EquipDefaults() {
	foundLootTypes.resize(0);

	AddFoundLoot( "standardlaser.json" );
	AddFoundLoot( "stablegrav.json" );

	SetCurrentWeapon( "standardlaser.json" );
	SetCurrentAntiGrav( "stablegrav.json" );
}

void PlayerData::AddFoundLoot( std::string name ) {
	if( std::find( foundLootTypes.begin(), foundLootTypes.end(), name ) == foundLootTypes.end() ) {
		foundLootTypes.push_back( name );
	}
}

void PlayerData::LoadLootTypes() {
	Json::Value root = app.GetJsonPool().GetRootNode( "../media/entity/loot/!lootindex.json" );

	for( int i = 0; i != root["LootFileNames"].size(); ++i ) {
		lootTypes.push_back( root["LootFileNames"][i].asCString() );
	}
}

void PlayerData::PopulateLootDetails() {
	for( int i = 0; i != lootTypes.size(); ++i ) {
		lootDetails[ lootTypes[i] ] = LootFromFile(lootTypes[i]);
	}

}

std::shared_ptr<Loot> PlayerData::LootFromFile( std::string file ) {
	Json::Value root = app.GetJsonPool().GetRootNode( "../media/entity/loot/" + file );

	Loot *theLoot = nullptr; // Will store different children of Loot, i.e WeaponLoot, EngineLoot etc.

	if( root["Category"].asString() == "Weapon" ) {
		WeaponLoot *weaponLoot = new WeaponLoot;

		weaponLoot->fireRate = root["FireRate"].asFloat();
		weaponLoot->projectileType = root["ProjectileType"].asString();

		theLoot = weaponLoot;
	} else if( root["Category"].asString() == "AntiGrav" ) {
		AntiGravLoot *antigravLoot = new AntiGravLoot;

		antigravLoot->gravityFactor = root["GravityFactor"].asFloat();

		theLoot = antigravLoot;
	}

	if( theLoot == nullptr ) {
		throw std::exception( std::string("Unrecognized loot category \'" + root["Category"].asString() + "\' in: " + file).c_str() );
	}

	theLoot->fileName = file;
	theLoot->category = root["Category"].asString();
	theLoot->realName = root["Name"].asString();
	theLoot->description = root["Description"].asString();

	std::cout << "Loot type loaded: " << theLoot->realName << std::endl;

	return std::shared_ptr<Loot>(theLoot);
}