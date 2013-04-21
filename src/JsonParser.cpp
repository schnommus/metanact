#include "JsonParser.h"

#include <fstream>

Json::Value &JsonParser::GetRootNode( std::string filename ) {
	// Only parse a file if it hasn't been parsed before
	if( cachedFiles.find(filename) == cachedFiles.end() ) {
		
		std::ifstream ifs;
		ifs.open(filename.c_str(), std::ios::in);

		if( !ifs.is_open() ) {
			throw std::exception(std::string("Couldn't open JSON file: " + filename).c_str());
		}

		if( !jsonReader.parse( ifs , cachedFiles[filename] ) ) {
			throw std::exception( std::string("Failed to parse JSON in " + filename + ". Errors: " + jsonReader.getFormattedErrorMessages() ).c_str() );
		}

		ifs.close();

		std::cout << "JSON file parsed: " << filename << std::endl;
	}

	// Hence or otherwise, return the cached version
	return cachedFiles[filename];
}