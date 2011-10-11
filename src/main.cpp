#include "config.h"
#include "ofMain.h"
#include "AppController.h"
#include "ofAppGlutWindow.h"
#include "Logger.h"

#ifdef RUN_IN_TEST_MODE
#include "TestController.h"
#endif
//========================================================================
int main( ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1280,720, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	LOGGER->setLogLevel(JU_LOG_VERBOSE);
	
#ifdef RUN_IN_TEST_MODE
	TestController * tc = new TestController(new AppController(&window));
	ofRunApp(tc->_appController);
#else
	ofRunApp( new AppController(&window));
#endif
}
