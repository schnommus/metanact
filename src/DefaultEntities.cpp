#include "DefaultEntities.h"
#include "MenuSystem.h"

bool SplashScreen::onStep(float delta) {
	if(!c) {
		// Allow skipping of splash screen on escape key
		if( app.GetInput().IsKeyDown(sf::Key::Escape) ) {
			c = true;
		}

		if( alpha < 250 && !f) {
			alpha += delta*100;
		} else {
			f = true;
		}

		if( f && alpha > 1) {
			alpha -= delta*100;
		} else if (f && alpha <= 1) {
			if(s) c = true;
			alpha = 0;
			s = true;
			f = false;
		}

		if(s)
			mets.SetColor(sf::Color(255, 255, 255, alpha));
		else
			schs.SetColor(sf::Color(255, 255, 255, alpha));
	} else {
		//app.inGame = true;
		app.AddEntity(new FPSMeter(app), 1001, true);
		app.AddEntity(new MenuSystem(app), 1001, true);
		app.RemoveEntity(this->id);
	}
	return true;
}