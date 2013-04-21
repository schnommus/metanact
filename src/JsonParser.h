#pragma once

#include "jsoncpp\json\json.h"
#include <string>
#include <map>

class JsonParser {
public:
	Json::Value &GetRootNode( std::string filename );
private:
	// Object that does the parsing
	Json::Reader jsonReader;

	// Map of filename to parsed root node
	std::map< std::string, Json::Value > cachedFiles;
};