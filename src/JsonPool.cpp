#include "JsonPool.h"

#include <fstream>

Json::Value &JsonPool::GetRootNode( std::string filename ) {
	// Only parse a file if it hasn't been parsed before
	if( cachedFiles.find(filename) == cachedFiles.end() ) {
		
		std::ifstream ifs;
		ifs.open(filename.c_str(), std::ios::in);

		if( !ifs.is_open() ) {
			std::cout << "Couldn't open JSON file: " <<  filename << ". Creating blank entry." << std::endl;
			cachedFiles[filename] = Json::Value();
		} else {
			if( !jsonReader.parse( ifs , cachedFiles[filename] ) ) {
				throw std::exception( std::string("Failed to parse JSON in " + filename + ". Errors: " + jsonReader.getFormattedErrorMessages() ).c_str() );
			}
			std::cout << "JSON file parsed: " << filename << std::endl;
		}

		ifs.close();
	}

	// Hence or otherwise, return the cached version
	return cachedFiles[filename];
}

void JsonPool::WriteToFile( std::string filename ) {
	if( cachedFiles.find(filename) == cachedFiles.end() ) {
		throw std::exception(std::string("Attempted to write file without data - " + filename).c_str());
	}

	std::ofstream ofs;
	ofs.open(filename.c_str(), std::ios::out | std::ios::trunc );

	if( !ofs.is_open() ) {
		throw std::exception(std::string("Couldn't open file for writing: " + filename).c_str());
	}

	ofs << cachedFiles[filename];
	ofs.close();

	std::cout << "Data serialized to JSON file: " << filename << std::endl;
}