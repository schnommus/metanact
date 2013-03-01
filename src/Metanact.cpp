#include "App2D.h"

int main() {
	std::string cp;
	std::cout << "Metanact v0.1a - (C) Seb Holzapfel 2012\nEnter initial directory: ";
	std::getline(std::cin, cp);

	// Init windowed, with vsync, low res when server (the various isServer parameters)
	// Look at App2D's constructor to see what I mean
	App2D a(true, true, "Metanact", true, "Seb", 800, 600 );

	a.currentPath = cp;

	a.Run();

    return 0;
}