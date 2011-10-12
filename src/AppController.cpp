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

    // need to comment out ofSoundStreamClose() in method ofExitCallback() in ofAppRunner -> some RTAUDIO bug wrecks clean exit!

    LOG_NOTICE("Saving properties");
#ifndef USE_DUMMY
#ifdef TARGET_WIN32
    // save cam props
	//_camControllers[0]->saveSettings();
	//_camControllers[1]->saveSettings();
#endif
#endif
	_dataController->saveProperties();

    delete _appView;

    delete _camControllers[0];
	delete _camControllers[1];
	delete _vidController;
	delete _dataController;
	delete _ardController;
	delete _micController;
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

	//_appModel->registerStates();
	_appModel->setState(kAPP_INIT);

    // load fake faces
    _appModel->getFakeVictimPlayer()->loadMovie("E:/gameoverload/VideoProjects/Jungle/finalALL/fake/SubstituteFace0.mov");
    _appModel->getFakeVictimPlayer()->play();
    _appModel->getFakeAttackPlayer()->loadMovie("E:/gameoverload/VideoProjects/Jungle/finalALL/fake/SubstituteFace1.mov");
    _appModel->getFakeAttackPlayer()->play();

	// set up datacontroller
	_dataController = new DataController(ofToDataPath("config_properties.xml"));

    // setup background sound controller
    _soundController = new SoundController();

	// setup videoController
	_vidController = new VideoController();
	//_vidController->registerStates();
    //ofSetLogLevel(OF_LOG_NOTICE);
	// setup micController
	_micController = new MicController("Microphone (Realtek High Definition Audio", 2); // other is Microphone (2- HD Pro Webcam C910) // TODO: make these a property
	ofAddListener(_micController->victimAction, this, &AppController::VictimEvent);
	//_micController->registerStates();

	// setup ardController
	_ardController = new ArdController("COM3", 1); // TODO: make this a property
	ofAddListener(_ardController->attackAction, this, &AppController::AttackEvent);
	//_ardController->registerStates();

	// setup cameras
	_camControllers[0] = new CamController();
	_camControllers[1] = new CamController();

#ifdef TARGET_OSX
	_camControllers[0]->setup("Built-in iSight", 640, 480);
	_camControllers[1]->setup("ManyCam Virtual Webcam (RGB)", 640, 480);	// NB: had to use QTKit to get ManyCam working
#else
	_camControllers[0]->setup(0, 1920, 1080);
	_camControllers[1]->setup(1, 1920, 1080);
#endif

//    ofAddListener(_camControllers[0]->faceAction, this, &AppController::FaceEvent);
//    ofAddListener(_camControllers[1]->faceAction, this, &AppController::FaceEvent);

	// register pointers to textures from cams on the model
	_appModel->setCameraTextures(_camControllers[0]->getCamTextureRef(), _camControllers[1]->getCamTextureRef());

	// set default for user action, temporary way to handle this stuff!
	_appModel->setProperty("userAction", kNoUserAction);

	// setup main app view
	_appView = new AppView(boost::any_cast<float>(_appModel->getProperty("appRenderWidth")),
						   boost::any_cast<float>(_appModel->getProperty("appRenderHeight")));


	_appModel->setState(kAPP_LOADING);

	// set app default properties - COMMENT TO STOP SETTING BACK TO DEFAULTS
	_appModel->setProperty("loadingMessage", string("AppController loading"));
	_appModel->setProperty("loadingProgress", 0.1f);
    //_appModel->setProperty("showCameras", false);
    _appModel->setProperty("showProps", false);
	_appModel->setProperty("fullScreen", false);
	_appModel->setProperty("tryScaleMethod", 0);
/*	_appModel->setProperty("cameraToAdjust", (string)"0");
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

    if (ofGetElapsedTimeMillis() -_appModel->getLastActionTime() > TIMEOUT_ACTION) {
        if (_switchToSequence == NULL && !_vidController->isPreRolling() && _appModel->getCurrentSequenceFrame() < _appModel->getCurrentSequenceNumFrames()-15) {
            if (_appModel->checkCurrentInteractivity(kINTERACTION_BOTH) || _appModel->checkCurrentInteractivity(kINTERACTION_VICTIM)) {
                _appModel->setLastActionTime(ofGetElapsedTimeMillis());
                string res = _appModel->getCurrentSequence()->getVictimResult()[_appModel->getCurrentSequenceFrame()];
                LOG_NOTICE("VICTIM ACTION [" + ofToString(level) + "] == " + res);
                if (res != kLAST_SEQUENCE_TOKEN) {
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

    if (ofGetElapsedTimeMillis() -_appModel->getLastActionTime() > TIMEOUT_ACTION) {
        if (_switchToSequence == NULL && !_vidController->isPreRolling() && _appModel->getCurrentSequenceFrame() < _appModel->getCurrentSequenceNumFrames()-15) {
            if (_appModel->checkCurrentInteractivity(kINTERACTION_BOTH) || _appModel->checkCurrentInteractivity(kINTERACTION_ATTACKER)) {
                _appModel->setLastActionTime(ofGetElapsedTimeMillis());
                string res = _appModel->getCurrentSequence()->getAttackerResult()[_appModel->getCurrentSequenceFrame()];
                LOG_NOTICE("ATTACK ACTION [" + ofToString(level) + "] == " + res);
                if (res != kLAST_SEQUENCE_TOKEN) {
                    _switchToSequence = _appModel->getCurrentScene()->getSequence(res);
                    _vidController->loadMovie(_switchToSequence, true);
                } else nextScene();
            } //else cout << "Clocked by type " << _appModel->getCurrentInteractivity() << " = " << kINTERACTION_ATTACKER << endl;
        } //else cout << "Blocked by null" << endl;
    } //else cout << "Blocked by time" << endl;

}

//--------------------------------------------------------------
//void AppController::FaceEvent(int & level) {
//    // level == instanceID == GONE; level == instanceID+2 == HERE;
//    LOG_VERBOSE("FACE ACTION..." + ofToString(level));
//
//    if (_appModel->checkCurrentInteractivity(kINTERACTION_FACE) && level > 1) {
//        if (_switchToSequence == NULL && !_vidController->isPreRolling()) {
//            // FACE APPEARS (level > 1) && FACE_INTERACTION set in Flash for seq00a
//            _appModel->setLastActionTime(ofGetElapsedTimeMillis());
//            string res = "seq01a"; // hack -> que send to seq01a
//            LOG_NOTICE("FACE ACTION [" + ofToString(level) + "] == " + res);
//            if (res != kLAST_SEQUENCE_TOKEN) {
//                _switchToSequence = _appModel->getCurrentScene()->getSequence(res);
//                _vidController->loadMovie(_switchToSequence);
//                _appModel->getCurrentVideoPlayer()->setLoopState(OF_LOOP_NONE);
//                _vidController->_preRolling = true;
//            } else nextScene();
//        }
//    } //else cout << "Blocked by null" << endl;
//
//
//}

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
			goThreadedVideo * movie;

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
			_appModel->setFacePresent(0,false);
			_appModel->setFacePresent(1,false);
			_appModel->setState(kAPP_RUNNING);
			_appModel->setLastActionTime(ofGetElapsedTimeMillis());

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
            _appModel->setLastActionTime(ofGetElapsedTimeMillis());
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

        // Auto action if no attacker present in loops -> uses a timeout
        if (!_appModel->getSwapFacePresent(1)) {
            if (_lastAutoAttackAction != 0 && ofGetElapsedTimeMillis() - _lastAutoAttackAction > TIMEOUT_AUTOATTACK) {
                LOG_NOTICE("Auto attacker action triggered");
                _lastAutoAttackAction = 0;
#ifdef DO_AUTO_TIMEOUT
                float level = 1.2;
                AttackEvent(level);
#endif
            } else if (_lastAutoAttackAction == 0 && currentSequence->getType() == "loop") {
                _lastAutoAttackAction = ofGetElapsedTimeMillis();
            } else if (_lastAutoAttackAction != 0 && currentSequence->getType() != "loop") {
                _lastAutoAttackAction = 0;
            }
        } else _lastAutoAttackAction = 0;

        // face arrive on seq00a waits TIMEOUT_AUTOFACE before going to seq01a
        if (currentSequence->getNumber() == 0) {
            if (_appModel->getAnyFacePresent()) {
                if (_lastAutoFaceAction != 0 && ofGetElapsedTimeMillis() - _lastAutoFaceAction > TIMEOUT_AUTOFACE) {
                    LOG_NOTICE("Auto face action triggered");
                    _switchToSequence = _appModel->getCurrentScene()->getSequence("seq01a");
                    _vidController->loadMovie(_switchToSequence, true);
                    //_appModel->getCurrentVideoPlayer()->setLoopState(OF_LOOP_NONE);
                    //_vidController->_preRolling = true;
                } else if (_lastAutoFaceAction == 0) _lastAutoFaceAction = ofGetElapsedTimeMillis();
            } else _lastAutoFaceAction = 0;
        } else _lastAutoFaceAction = 0;

//        // if faces are already present when a new scene starts wait till the end of the scene and jump to seq01a
//        if (currentSequence->getNumber() == 0 && _appModel->getAnyFacePresent() && _appModel->getCurrentSequenceFrame() >= _appModel->getCurrentSequenceNumFrames()-4) {
//            LOG_NOTICE("Auto start scene " + ofToString(_appModel->getCurrentSequenceFrame()) + " " + ofToString(_appModel->getCurrentSequenceNumFrames()-4));
//#ifdef DO_AUTO_TIMEOUT
//            int level = 2;
//            FaceEvent(level);
//#endif
//        }

        // if no faces present after timeout and no user action for more than timeout we can push to next scene
        if (currentSequence->getNumber() > 0 && !_appModel->getAnyFacePresent() && ofGetElapsedTimeMillis() -_appModel->getLastActionTime() > TIMEOUT_ACTION) {
#ifdef DO_AUTO_TIMEOUT
            LOG_NOTICE("Auto No Face present for more than " + ofToString(TIMEOUT_NOFACE) + " forcing next scene");
            nextScene();
#endif
        }
	}

}

void AppController::nextScene() {

    LOG_VERBOSE("Current scene ended, rewind current scene to first sequence. Loading next scene.");

    Scene* currentScene = _appModel->getCurrentScene();
    if (currentScene != NULL) currentScene->rewindScene();

    _appModel->nextScene();
    _vidController->reset();

    currentScene = _appModel->getCurrentScene();

    _camControllers[0]->loadAttributes();
    _camControllers[1]->loadAttributes();

     if (_appModel->getAnyFacePresent()) {
         _switchToSequence = _appModel->getCurrentScene()->getSequence("seq01a");
     } else _switchToSequence = currentScene->getCurrentSequence();

    _soundController->setup();
    _soundController->setVolume(1.0);

    _vidController->loadMovie(_switchToSequence, true);

}

//--------------------------------------------------------------
void AppController::draw() {
	//	LOG_VERBOSE("Drawing");
	ofSetColor(255, 255, 255, 255);
	_appView->draw();

#ifdef DEBUG_VIEW_ENABLED
    if (boost::any_cast<bool>(_appModel->getProperty("showCameras"))) {
	    _camControllers[0]->drawDebug(ofGetWidth() - 640.0f/4.0f, 20.0f, 640.0f/4.0f, 640.0f/4.0f);
        _camControllers[1]->drawDebug(ofGetWidth() - 640.0f/4.0f + 640.0f, 0.0f, 640.0f/4.0f, 640.0f/4.0f);
    }
#endif

}

//--------------------------------------------------------------
void AppController::keyPressed(int key){

    LOG_VERBOSE("Key pressed: " + (char)(key));

    PosRotScale * prsToAdjust[2];

    if (_appModel->getSwapFacePresent(0)) {
        prsToAdjust[0] = _appModel->getCameraAttributes(0);
    } else {
        prsToAdjust[0] = _appModel->getFakeAttributes(0);
    }

    if (_appModel->getSwapFacePresent(1)) {
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
			_micController->fakeVictimAction(fakeInput*32.0f);
			break;
		case 'p':
			_ardController->fakeAttackAction(fakeInput);
			break;
		case 'S':
			_dataController->saveProperties();
			break;
		case ' ':
            _switchToSequence = _appModel->getCurrentScene()->getSequence("seq01a");
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
            _appModel->getCurrentVideoPlayer()->togglePaused();
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
             _camControllers[0]->_doFaceTracking ^= true;
             _camControllers[1]->_doFaceTracking ^= true;
             _camControllers[0]->_doFaceDetection ^= true;
             _camControllers[1]->_doFaceDetection ^= true;
            break;
        case 'j':
             nextScene();
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

    _appModel->setProperty("fullScreen", !boost::any_cast<bool>(_appModel->getProperty("fullScreen")));

}
