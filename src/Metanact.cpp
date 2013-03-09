#include "App2D.h"

int main() {
	std::cout << "Metanact v0.4a - (C) Seb Holzapfel 2012\n";

	// Init windowed, with vsync, low res when server (the various isServer parameters)
	// Look at App2D's constructor to see what I mean
	App2D a(false, true, "Metanact", true, "Seb", 800, 600 );

	a.Run();

    return 0;
}