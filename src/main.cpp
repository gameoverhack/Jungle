#include "ofMain.h"
#include "AppController.h"
#include "ofAppGlutWindow.h"
#include "Logger.h"

//========================================================================
int main( ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1280,720, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	LOGGER->setLogLevel(JU_LOG_VERBOSE);
	ofRunApp( new AppController(&window));

}
