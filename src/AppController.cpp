/*
 *  AppController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "AppController.h"
#include "Logger.h"

//--------------------------------------------------------------
AppController::AppController(ofAppGlutWindow * windowPtr) {
	//nothing for now (NB: I put the Logger instantiation in main.h)
	_windowPtr = windowPtr;
}

//--------------------------------------------------------------
AppController::~AppController() {

}

void AppController::exit(){
    // need to comment out ofSoundStreamClose() in method ofExitCallback() in ofAppRunner -> some RTAUDIO bug wrecks clean exit!
    LOG_NOTICE("Saving properties");
    _appModel->setState(kAPP_EXITING);

	_dataController->saveProperties();

    delete _appView;

    delete _camControllers[0];
	delete _camControllers[1];
	delete _vidController;
	delete _dataController;
	delete _micController;
	delete _appModel;
	delete _ardController;
	cout << "here" << endl;
}

//--------------------------------------------------------------
void AppController::setup() {

	LOG_NOTICE("Initialising");
    ofSetLogLevel(OF_LOG_VERBOSE);

	ofSetFrameRate(60);          // has to be as we can't set hammer ofArduino too hard...hmmm
	ofSetVerticalSync(true);
    //ofSetDrawBitmapMode(OF_BITMAPMODE_SCREEN);

	_isFullScreen		= false; // change this when we start in fullscreen mode
	_switchToSequence	= NULL;

	_appModel->setState(kAPP_INIT);

    // load fake faces
#ifdef USER_MATT
    // TODO:  this doesnt seem to be used anywhere any more...? It gets loaded -> play() -> update() but the pixels are never drawn anywhere.
    //          only references to fakevictimplayer is in app controller (here and in update)()) for the above described calls.
    _appModel->getFakeVictimPlayer()->loadMovie("E:/gameoverload/VideoProjects/Jungle/finalALL/fake/SubstituteFace0.mov");
    _appModel->getFakeVictimPlayer()->play();
    _appModel->getFakeAttackPlayer()->loadMovie("E:/gameoverload/VideoProjects/Jungle/finalALL/fake/SubstituteFace1.mov");
    _appModel->getFakeAttackPlayer()->play();
#else
    _appModel->getFakeVictimPlayer()->loadMovie("D:/finalALL/fake/SubstituteFace0.mov");
    _appModel->getFakeVictimPlayer()->play();
    _appModel->getFakeAttackPlayer()->loadMovie("D:/finalALL/fake/SubstituteFace1.mov");
    _appModel->getFakeAttackPlayer()->play();
#endif

	// set up datacontroller
	_dataController = new DataController(ofToDataPath("config_properties.xml"));

    // setup background sound controller
    _soundController = new SoundController();

	// setup videoController
	_vidController = new VideoController();

	// setup micController
	// NOTE ACTUALLY JUST USING DEFAULT DEVICE SO WE CAN SET THIS THROUGH THE OPERATING SYSTEM!
	_micController = new MicController("Microphone (Realtek High Definition Audio", 2); // other is Microphone (2- HD Pro Webcam C910) // TODO: make these a property
	ofAddListener(_appModel->victimAction, this, &AppController::VictimEvent);

	// setup ardController
	_ardController = new ArdController("COM3", 1); // TODO: make this a property
	ofAddListener(_appModel->attackAction, this, &AppController::AttackEvent);

	// setup cameras
	_camControllers[0] = new CamController();
	_camControllers[1] = new CamController();

#ifdef TARGET_OSX
	_camControllers[0]->setup("Built-in iSight", 640, 480);
	_camControllers[1]->setup("ManyCam Virtual Webcam (RGB)", 640, 480);	// NB: had to use QTKit to get ManyCam working
#else
	if(!_camControllers[0]->setup(1, 1920, 1080)){
		LOG_ERROR("Could not setup camController[0]");
		abort();
	}
	if(!_camControllers[1]->setup(0, 1920, 1080)){
		LOG_ERROR("Could not setup camController[1]");
		abort();
	}
#endif

//    ofAddListener(_camControllers[0]->faceAction, this, &AppController::FaceEvent);
//    ofAddListener(_camControllers[1]->faceAction, this, &AppController::FaceEvent);

	// register pointers to textures from cams on the model
	_appModel->setCameraTextures(_camControllers[0]->getCamTextureRef(), _camControllers[1]->getCamTextureRef());

	// set default for user action, temporary way to handle this stuff!
	_appModel->setProperty("userAction", kNoUserAction);

	// set app default properties - COMMENT TO STOP SETTING BACK TO DEFAULTS
	_appModel->setProperty("loadingMessage", string("AppController loading"));
	_appModel->setProperty("loadingProgress", 0.1f);
    //_appModel->setProperty("showCameras", false);
//    _appModel->setProperty("showProps", false);
	_appModel->setProperty("fullScreen", false);
//	_appModel->setProperty("tryScaleMethod", 0);
//	_appModel->setProperty("anyActionTimer", 500);
//	_appModel->setProperty("victimActionTimer", 3000);
//	_appModel->setProperty("attackActionTimer", 3000);
//    _appModel->setProperty("anyFaceTimer", 10000);
//	_appModel->setProperty("victimFaceTimer", 5000);
//	_appModel->setProperty("attackFaceTimer", 5000);
//	_appModel->setProperty("autoAttackTimer", 5000);
/*	_appModel->setProperty("ardAttackMin", 300.0f);
	_appModel->setProperty("ardAttackMax", 800.0f);
	_appModel->setProperty("cameraToAdjust", (string)"0");
	_appModel->setProperty("cameraPropToAdjust", (string)"SCALEROTATION");
    _appModel->setProperty("camScale0", 0.0f);
	_appModel->setProperty("camScale1", 0.0f);
	_appModel->setProperty("camRotation0", 0.0f);
	_appModel->setProperty("camRotation1", 0.0f);
    _appModel->setProperty("camPositionX0", 0.0f);
	_appModel->setProperty("camPositionX1", 0.0f);
	_appModel->setProperty("camPositionY0", 0.0f);
	_appModel->setProperty("camPositionY1", 0.0f);

    _appModel->setProperty("showFFT", false);*/

    // create timer for any interaction
    Timer * anyActionTimer = new Timer(boost::any_cast<int>(_appModel->getProperty("anyActionTimer")));
    _appModel->addTimer("anyActionTimer", anyActionTimer);
    _appModel->startTimer("anyActionTimer");

    Timer * victimActionTimer = new Timer(boost::any_cast<int>(_appModel->getProperty("victimActionTimer")));
    _appModel->addTimer("victimActionTimer", victimActionTimer);
    _appModel->startTimer("victimActionTimer");

    Timer * attackActionTimer = new Timer(boost::any_cast<int>(_appModel->getProperty("attackActionTimer")));
    _appModel->addTimer("attackActionTimer", attackActionTimer);
    _appModel->startTimer("attackActionTimer");

    Timer * anyFaceTimer = new Timer(boost::any_cast<int>(_appModel->getProperty("anyFaceTimer")));
    _appModel->addTimer("anyFaceTimer", anyFaceTimer);

    Timer * victimFaceTimer = new Timer(boost::any_cast<int>(_appModel->getProperty("victimFaceTimer")));
    _appModel->addTimer("victimFaceTimer", victimFaceTimer);

    Timer * attackFaceTimer = new Timer(boost::any_cast<int>(_appModel->getProperty("attackFaceTimer")));
    _appModel->addTimer("attackFaceTimer", attackFaceTimer);


    Timer * autoAttackTimer = new Timer(boost::any_cast<int>(_appModel->getProperty("autoAttackTimer")));
    _appModel->addTimer("autoAttackTimer", autoAttackTimer);
    //_appModel->startTimer("autoAttackTimer");

    	// setup main app view
	_appView = new AppView(boost::any_cast<float>(_appModel->getProperty("appRenderWidth")),
						   boost::any_cast<float>(_appModel->getProperty("appRenderHeight")));

	_appModel->setState(kAPP_LOADING);

    toggleFullscreen();
    ofHideCursor();

	LOG_NOTICE("Initialisation complete");
}

//--------------------------------------------------------------
void AppController::swapCameras() {
	// use pointer swap and re-register texture references on the model
	swap(_camControllers[0], _camControllers[1]);
	_appModel->setCameraTextures(_camControllers[0]->getCamTextureRef(), _camControllers[1]->getCamTextureRef());
}

//--------------------------------------------------------------
void AppController::VictimEvent(float & level) {

    if (_appModel->hasTimedOut("anyActionTimer")) {
        if (_switchToSequence == NULL && !_vidController->isPreRolling() && _appModel->getCurrentSequenceFrame() < _appModel->getCurrentSequenceNumFrames()-15) {
            if (_appModel->checkCurrentInteractivity(kINTERACTION_BOTH) || _appModel->checkCurrentInteractivity(kINTERACTION_VICTIM)) {
                _appModel->restartTimer("anyActionTimer");
                string res = _appModel->getCurrentSequence()->getVictimResult()[_appModel->getCurrentSequenceFrame()];
                LOG_NOTICE("VICTIM ACTION [" + ofToString(level) + "] == " + res);
                if (res != kLAST_SEQUENCE_TOKEN && res != "") {
                    _switchToSequence = _appModel->getCurrentScene()->getSequence(res);
                    int returnFrame = _appModel->getCurrentSequenceFrame() + 1;
                    if (returnFrame > _appModel->getCurrentSequenceNumFrames()) returnFrame -= 15;
                    _vidController->loadMovie(_switchToSequence, true, returnFrame);
                } else nextScene();

            } //else cout << "Clocked by type " << _appModel->getCurrentInteractivity() << " = " << kINTERACTION_VICTIM << endl;
        } //else cout << "Blocked by null" << endl;
    } //else cout << "Blocked by time" << endl;

}

//--------------------------------------------------------------
void AppController::AttackEvent(float & level) {

    if (_appModel->hasTimedOut("anyActionTimer")) {
        if (_switchToSequence == NULL && !_vidController->isPreRolling() && _appModel->getCurrentSequenceFrame() < _appModel->getCurrentSequenceNumFrames()-15) {
            if (_appModel->checkCurrentInteractivity(kINTERACTION_BOTH) || _appModel->checkCurrentInteractivity(kINTERACTION_ATTACKER)) {
                _appModel->restartTimer("anyActionTimer");
                string res = _appModel->getCurrentSequence()->getAttackerResult()[_appModel->getCurrentSequenceFrame()];
                LOG_NOTICE("ATTACK ACTION [" + ofToString(level) + "] == " + res);
                if (res != kLAST_SEQUENCE_TOKEN && res != "") {
                    _switchToSequence = _appModel->getCurrentScene()->getSequence(res);
                    _vidController->loadMovie(_switchToSequence, true);
                } else nextScene();
            } //else cout << "Clocked by type " << _appModel->getCurrentInteractivity() << " = " << kINTERACTION_ATTACKER << endl;
        } //else cout << "Blocked by null" << endl;
    } //else cout << "Blocked by time" << endl;

}

//--------------------------------------------------------------
void AppController::update() {


    _micController->update();
    _ardController->update();

	// if we're loading, update datacontroller
	if(_appModel->checkState(kAPP_LOADING)){

		_dataController->update();
        _appView->update();

		if(_dataController->checkState(kDATACONTROLLER_FINISHED)){

			// DC finished...let's load the first movie
			Scene			* currentScene;
			ofxThreadedVideo * movie;

			// get current scene
			_appModel->setCurrentScene("t");
			currentScene = _appModel->getCurrentScene();

			// force load using _switchToSequence var which will be caught below when the movie is fully loaded...
			_switchToSequence = currentScene->getCurrentSequence();
            _camControllers[0]->loadAttributes();
            _camControllers[1]->loadAttributes();
            _soundController->setup();
			_soundController->loadSound();
			_vidController->loadMovie(_switchToSequence, true);
			_appModel->setState(kAPP_RUNNING);
			_appModel->restartTimer("anyActionTimer");
			_appModel->startTimer("anyFaceTimer");
            _appModel->startTimer("victimFaceTimer");
            _appModel->startTimer("attackFaceTimer");
            _appModel->startTimer("autoAttackTimer");
		}
	}

	// running, so update scenes, etc
	if(_appModel->checkState(kAPP_RUNNING) && ((_ardController->checkState(kARDCONTROLLER_READY) || _ardController->checkState(kARDCONTROLLER_DISABLED)))) {

        _camControllers[0]->update();
        _camControllers[1]->update();

        _vidController->update();
        _soundController->update();

        _appModel->getFakeAttackPlayer()->update();
        _appModel->getFakeVictimPlayer()->update();

        _appView->update();

		Scene			* currentScene		= _appModel->getCurrentScene();
		Sequence		* currentSequence	= currentScene->getCurrentSequence();

		if (_vidController->checkState(kVIDCONTROLLER_NEXTVIDERROR) && _switchToSequence != NULL) {
			LOG_VERBOSE("ERROR on load. Try again? This is super-inelegant but does seem to let us keep running");
			_vidController->loadMovie(_switchToSequence, true);
		}

		if (_switchToSequence == NULL && _vidController->checkState(kVIDCONTROLLER_CURRENTVIDONE)) {
			// the video just finished and we toggled to next video if there is one
            LOG_VERBOSE("Trying to go to next sequence");

			if(currentScene->nextSequence()) {
				// loaded next sequence in this scene, keep going
				LOG_VERBOSE("Gone to next sequence");
				_appModel->restartTimer("anyActionTimer");
				_appModel->restartTimer("victimActionTimer");
				_appModel->restartTimer("attackActionTimer");
				_appModel->restartTimer("autoAttackTimer");
				_vidController->setState(kVIDCONTROLLER_READY);
				_soundController->setVolume(1.0);
				_appView->update();
			} else {
                nextScene();
			}

		}

		// catch _switchToSequence when a movie is loaded completely
		if (_switchToSequence != NULL && _vidController->checkState(kVIDCONTROLLER_NEXTVIDREADY)) {
            LOG_NOTICE("Doing switchMovie on NEXTVIDEOREADY");
            _appModel->restartTimer("anyActionTimer");
            _appModel->restartTimer("victimActionTimer");
            _appModel->restartTimer("attackActionTimer");
            _appModel->restartTimer("autoAttackTimer");
            _vidController->toggleVideoPlayers();
            _appView->update();
            _vidController->setState(kVIDCONTROLLER_READY);
            _soundController->setVolume(1.0);
            currentScene->setCurrentSequence(_switchToSequence);
            _switchToSequence = NULL;
		}


#ifdef RUN_IN_TEST_MODE
		// throw fake update event, float is bogas.
		ofNotifyEvent(updateEvent, 1.0, this);
#endif

        _appModel->getFakeAttackPlayer()->update();
        _appModel->getFakeVictimPlayer()->update();

        if(_appModel->hasTimedOut("attackFaceTimer")){
            if(_appModel->hasTimedOut("autoAttackTimer") && (_appModel->getCurrentInteractivity() == kINTERACTION_ATTACKER || _appModel->getCurrentInteractivity() == kINTERACTION_BOTH)) {
                    LOG_NOTICE("Auto attacker action triggered");
                    _appModel->restartTimer("autoAttackTimer");
                    float level = 8.0;
                    AttackEvent(level);
            }

        }else{
            _appModel->restartTimer("autoAttackTimer");
        }

    if (currentSequence->getNumber() == 0 && (_appModel->getPinInput()[2] > 100 || _appModel->getPinInput()[3] > 100)  && _appModel->hasTimedOut("anyActionTimer") && _switchToSequence == NULL){
        LOG_NOTICE("Found a face! Lets goooooooo....");
        _switchToSequence = _appModel->getCurrentScene()->getSequence("seq01a");
        _vidController->loadMovie(_switchToSequence, true);
    }

#ifdef DO_AUTO_TIMEOUT
        // if no faces present after timeout and no user action for more than timeout we can push to next scene
        if (currentSequence->getNumber() > 1 && currentSequence->getNumber() < 8 && _appModel->hasTimedOut("anyFaceTimer") && _appModel->hasTimedOut("anyActionTimer") && _switchToSequence == NULL) {
            LOG_NOTICE("Auto No Face present for more than " + ofToString(TIMEOUT_NOFACE) + " forcing next scene");
            nextScene();
        }
#endif
	}

}

void AppController::nextScene() {

    if (_vidController->checkState(kVIDCONTROLLER_READY) || _vidController->checkState(kVIDCONTROLLER_CURRENTVIDONE)) {

        LOG_VERBOSE("Current scene ended, rewind current scene to first sequence. Loading next scene.");

        Scene* currentScene = _appModel->getCurrentScene();
        if (currentScene != NULL) currentScene->rewindScene();

        _appModel->nextScene();
        _vidController->reset();

        currentScene = _appModel->getCurrentScene();

        _camControllers[0]->loadAttributes();
        _camControllers[1]->loadAttributes();

         if (_appModel->getPinInput()[2] > 100 || _appModel->getPinInput()[3] > 100) {
             _switchToSequence = _appModel->getCurrentScene()->getSequence("seq01a");
         } else _switchToSequence = _appModel->getCurrentScene()->getSequence("seq00a");

        _soundController->setup();
        _soundController->setVolume(1.0);

        _vidController->loadMovie(_switchToSequence, true);
    }
}

//--------------------------------------------------------------
void AppController::draw() {
	//	LOG_VERBOSE("Drawing");
	ofSetColor(255, 255, 255, 255);
	_appView->draw();

#ifdef DEBUG_VIEW_ENABLED
    if (boost::any_cast<bool>(_appModel->getProperty("showCameras"))) {
	    _camControllers[0]->drawDebug(0, 0, 1920.0f/4.0f, 1080.0f/4.0f);
        _camControllers[1]->drawDebug(1920.0f/4.0f, 0.0f, 1920.0f/4.0f, 1080.0f/4.0f);
    }
#endif

}

//--------------------------------------------------------------
void AppController::keyPressed(int key){

    LOG_VERBOSE("Key pressed: " + ofToString(key));

    PosRotScale * prsToAdjust[2];

    if (_appModel->hasTimedOut("victimFaceTimer")) {
        prsToAdjust[0] = _appModel->getCameraAttributes(0);
    } else {
        prsToAdjust[0] = _appModel->getFakeAttributes(0);
    }

    if (_appModel->hasTimedOut("attackFaceTimer")) {
        prsToAdjust[1] = _appModel->getCameraAttributes(1);
    } else {
        prsToAdjust[1] = _appModel->getFakeAttributes(1);
    }

	float gamma                 = boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection"));
	float blend                 = boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio"));
	bool showUnmask             = boost::any_cast<bool>(_appModel->getProperty("showUnmaskedTextures"));
    bool showProps              = boost::any_cast<bool>(_appModel->getProperty("showProps"));
    bool showFFT                = boost::any_cast<bool>(_appModel->getProperty("showFFT"));
    bool showDebug              = boost::any_cast<bool>(_appModel->getProperty("showDebugView"));
    bool showCameras            = boost::any_cast<bool>(_appModel->getProperty("showCameras"));
    int scaleMethod             = boost::any_cast<int>(_appModel->getProperty("tryScaleMethod"));
    int cameraToAdjust          = boost::any_cast<int>(_appModel->getProperty("cameraToAdjust"));
    string cameraPropToAdjust   = boost::any_cast<string>(_appModel->getProperty("cameraPropToAdjust"));

    float rotationAdjustment    = 0.5f;
    float scaleAdjustment       = 0.01f;
    float positionAdjustment    = 5.0f;

    float fakeInput             = 1024.0f; // this could be faked more realistically using time between keys etc -> will do for now

	switch (key) {
#ifndef USE_DUMMY
#ifdef TARGET_WIN32
	    case '3':
			_camControllers[0]-> showVideoSettings();
			break;
		case '4':
			_camControllers[1]-> showVideoSettings();
			break;
        case 'c':
			_camControllers[0]->saveSettings();
			_camControllers[1]->saveSettings();
            _camControllers[0]->saveAttributes();
			_camControllers[1]->saveAttributes();
			break;
        case 'v':
			_camControllers[0]->loadSettings();
			_camControllers[1]->loadSettings();
			break;
#endif
#endif
        case 'b':
            _appModel->setProperty("showCameras", !showCameras);
            break;
        case 'n':
			swapCameras();
			break;
        case 'y':
            if (scaleMethod == 0) {
                 _appModel->setProperty("tryScaleMethod", 1);
            } else  _appModel->setProperty("tryScaleMethod", 0);
            break;
		case 'x':
			gamma += 0.1;
			break;
		case 'z':
			gamma -= 0.1;
			break;
		case 's':
			blend += 0.1;
			break;
		case 'a':
			blend -= 0.1;
			break;
		case 'd':
			_appModel->setProperty("showDebugView", !showDebug);
			break;
		case 'q':
			_micController->fakeVictimAction(ofRandom(fakeInput*32.0f));
			break;
		case 'p':
			_ardController->fakeAttackAction(ofRandom(fakeInput));
			break;
		case 'S':
			_dataController->saveProperties();
			break;
		case ' ':
//            _appModel->restartTimer("victimFaceTimer");
//            _appModel->restartTimer("anyFaceTimer");
//            _appModel->restartTimer("attackFaceTimer");
            _switchToSequence = _appModel->getCurrentScene()->getSequence("seq08a");
            _vidController->loadMovie(_switchToSequence, true);
            break;
		case '[':
            _soundController->fade(1.0, 1000, FADE_LOG);
            break;
        case ']':
            _soundController->fade(0.0, 2000, FADE_LOG);
            break;
        case '\\':
            _soundController->fade(0.0, 2000, FADE_EXP);
            break;
        case 'm':
            _appModel->getCurrentVideoPlayer()->setPaused(!_appModel->getCurrentVideoPlayer()->isPaused());
            break;
        case '/':
            _appModel->getCurrentVideoPlayer()->setFrame(_appModel->getCurrentSequenceNumFrames()-13);
            break;
		case '.':
            _appModel->getCurrentVideoPlayer()->nextFrame();
            break;
		case ',':
            _appModel->getCurrentVideoPlayer()->previousFrame();
			break;
        case '1':
            _appModel->setProperty("cameraToAdjust", 0);
            break;
        case '2':
            _appModel->setProperty("cameraToAdjust", 1);
            break;
        case '5':
            _appModel->setProperty("cameraPropToAdjust", (string)"POSITION");
            break;
        case '6':
            _appModel->setProperty("cameraPropToAdjust", (string)"SCALEROTATION");
            break;
		case 356: // left arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                prsToAdjust[cameraToAdjust]->r -= rotationAdjustment;

            }
            if (cameraPropToAdjust == "POSITION") {
                prsToAdjust[cameraToAdjust]->x -= positionAdjustment;
            }
			break;
		case 358: // right arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                prsToAdjust[cameraToAdjust]->r += rotationAdjustment;
            }
            if (cameraPropToAdjust == "POSITION") {
                prsToAdjust[cameraToAdjust]->x += positionAdjustment;
            }
			break;
        case 359: // up arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                prsToAdjust[cameraToAdjust]->s -= scaleAdjustment;
            }
            if (cameraPropToAdjust == "POSITION") {
                prsToAdjust[cameraToAdjust]->y -= positionAdjustment;
            }
			break;
		case 357: // down arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                prsToAdjust[cameraToAdjust]->s += scaleAdjustment;
            }
            if (cameraPropToAdjust == "POSITION") {
                prsToAdjust[cameraToAdjust]->y += positionAdjustment;
            }
			break;
        case 'r':
            prsToAdjust[cameraToAdjust]->x = 0.0f;
            prsToAdjust[cameraToAdjust]->y = 0.0f;
            prsToAdjust[cameraToAdjust]->s = 0.5f;
            prsToAdjust[cameraToAdjust]->r = 0.0f;
            break;
		case 'h':
			_appModel->setProperty("showUnmaskedTextures", !showUnmask);
			break;
		case 'f':
			toggleFullscreen();
			break;
        case '9':
			_appModel->setProperty("showFFT", !showFFT);
			break;
        case '0':
             _appModel->setProperty("showProps", !showProps);
            break;
        case 'w':

            break;
        case 'j':
             nextScene();
            break;
        case '7':
            ofHideCursor();
            break;
        case '8':
            ofShowCursor();
            break;
		default:
			break;
	}

	_appModel->setProperty("shaderBlendRatio", blend);
	_appModel->setProperty("shaderGammaCorrection", gamma);

}

//--------------------------------------------------------------
void AppController::keyReleased(int key){

}

//--------------------------------------------------------------
void AppController::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void AppController::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void AppController::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void AppController::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void AppController::windowResized(int w, int h){

}

void AppController::setWindowed() {

}

void AppController::setFullscreen() {

}

void AppController::toggleFullscreen(){
    /*
#ifdef TARGET_WIN32

    if (!boost::any_cast<bool>(_appModel->getProperty("fullScreen"))) {
        int currentWidth = ofGetWidth();
        LOG_VERBOSE("Trying to force fullscreen on Windows 7: " + ofToString(currentWidth));
        _windowTitle = "Jungle";
        ofSetWindowTitle(_windowTitle);
        int x = 0;
        int y = 0;
#ifdef EXTENDED_DISPLAY
        int width = _windowPtr->getScreenSize().x*2; // TODO: set in config
#else
        int width = _windowPtr->getScreenSize().x; // TODO: set in config
#endif
        int height = _windowPtr->getScreenSize().y; // TODO: set in config
        LOG_NOTICE("trying: " + ofToString(width) + " X " + ofToString(height));
        int storedWindowX, storedWindowY, storedWindowH, storedWindowW;
        HWND vWnd  = FindWindow(NULL, _windowTitle);
        long windowStyle = GetWindowLong(vWnd, GWL_STYLE);
        windowStyle &= ~WS_OVERLAPPEDWINDOW;
        windowStyle |= WS_POPUP;
        SetWindowLong(vWnd, GWL_STYLE, windowStyle);
        SetWindowPos(vWnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
        //_isFullScreen = true;
    } else {
        int x = 0;
        int y = 0;
        int width = 1280; // TODO: set in config
        int height = 720; // TODO: set in config
        HWND vWnd  = FindWindow(NULL,  _windowTitle);
        long windowStyle = GetWindowLong(vWnd, GWL_STYLE);
        windowStyle |= WS_TILEDWINDOW;
        SetWindowLong(vWnd, GWL_STYLE, windowStyle);
        SetWindowPos(vWnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
        //_isFullScreen = false;
    }
#else
    ofToggleFullscreen();
#endif
*/

    ofToggleFullscreen();

    _appModel->setProperty("fullScreen", !boost::any_cast<bool>(_appModel->getProperty("fullScreen")));

}
