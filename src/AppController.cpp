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
AppController::AppController(ofAppBaseWindow * windowPtr) {
	//nothing for now (NB: I put the Logger instantiation in main.h)
	_windowPtr = windowPtr;
}

//--------------------------------------------------------------
AppController::~AppController() {

    // need to comment out ofSoundStreamClose() in method ofExitCallback() in ofAppRunner -> some RTAUDIO bug wrecks clean exit!

    LOG_NOTICE("Saving properties");

#ifdef TARGET_WIN32
    // save cam props
	_camControllers[0]->saveSettings();
	_camControllers[1]->saveSettings();
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

	ofSetFrameRate(60);          // has to be as we can't set hammer ofArduino too hard...hmmm
	ofSetVerticalSync(true);

	_isFullScreen		= false; // change this when we start in fullscreen mode
	_switchToSequence	= NULL;

	//_appModel->registerStates();
	_appModel->setState(kAPP_INIT);

	//_flashAnalyzer->registerStates();

	// set up datacontroller
	_dataController = new DataController(ofToDataPath("config_properties.xml"));
	//_dataController->registerStates();

	// setup videoController
	_vidController = new VideoController();
	//_vidController->registerStates();

	// setup micController
	_micController = new MicController("Microphone (HD Pro Webcam C910)", 2); // other is Microphone (2- HD Pro Webcam C910) // TODO: make these a property
	ofAddListener(_micController->victimAction, this, &AppController::VictimEvent);
	//_micController->registerStates();

	// setup ardController
	_ardController = new ArdController("COM5", 2); // TODO: make this a property
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

    _appModel->setProperty("showProps", false);
	_appModel->setProperty("autoTest", false);
	_appModel->setProperty("fullScreen", true);
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

    if (ofGetElapsedTimeMillis() - _lastActionTime > 500) {
        if (_switchToSequence == NULL && !_vidController->isPreRolling()) {
            if (_appModel->checkCurrentInteractivity(kINTERACTION_BOTH) || _appModel->checkCurrentInteractivity(kINTERACTION_VICTIM)) {
                _lastActionTime = ofGetElapsedTimeMillis();
                string res = _appModel->getCurrentSequence()->getVictimResult();
                LOG_NOTICE("VICTIM ACTION [" + ofToString(level) + "]" + res);
                if (res != kLAST_SEQUENCE_TOKEN) {
                    _switchToSequence = _appModel->getCurrentScene()->getSequence(res);
                    _vidController->loadMovie(_switchToSequence, true, _appModel->getCurrentFrame()+1);
                } else nextScene();

            } //else cout << "Clocked by type " << _appModel->getCurrentInteractivity() << " = " << kINTERACTION_VICTIM << endl;
        } //else cout << "Blocked by null" << endl;
    } //else cout << "Blocked by time" << endl;

}

//--------------------------------------------------------------
void AppController::AttackEvent(float & level) {

    if (ofGetElapsedTimeMillis() - _lastActionTime > 500) {
        if (_switchToSequence == NULL && !_vidController->isPreRolling()) {
            if (_appModel->checkCurrentInteractivity(kINTERACTION_BOTH) || _appModel->checkCurrentInteractivity(kINTERACTION_ATTACKER)) {
                _lastActionTime = ofGetElapsedTimeMillis();
                string res = _appModel->getCurrentSequence()->getAttackerResult();
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
void AppController::FaceEvent(float & level) {

    if (ofGetElapsedTimeMillis() - _lastActionTime > 500) {
        if (_switchToSequence == NULL && !_vidController->isPreRolling()) {
            if (_appModel->checkCurrentInteractivity(kINTERACTION_FACE)) {
                _lastActionTime = ofGetElapsedTimeMillis();
                string res ="seq01a"; // hack
                LOG_NOTICE("FAKE FACE ACTION [" + ofToString(level) + "] == " + res);
                if (res != kLAST_SEQUENCE_TOKEN) {
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

	_appView->update();

	// if we're loading, update datacontroller
	if(_appModel->checkState(kAPP_LOADING)){

		_dataController->update();

		if(_dataController->checkState(kDATACONTROLLER_FINISHED)){

			// DC finished...let's load the first movie
			Scene			* currentScene;
			goThreadedVideo * movie;

			// get current scene
			_appModel->setCurrentScene("t");
			currentScene = _appModel->getCurrentScene();

			// force load using _switchToSequence var which will be caught below when the movie is fully loaded...
			_switchToSequence = currentScene->getCurrentSequence();
			_vidController->loadMovie(_switchToSequence, true);
			_appModel->setState(kAPP_RUNNING);
			_lastAutoActionTime = ofGetElapsedTimeMillis();

		}
	}

	// running, so update scenes, etc
	if(_appModel->checkState(kAPP_RUNNING) && (_ardController->checkState(kARDCONTROLLER_READY) || _ardController->checkState(kARDCONTROLLER_DISABLED))) {

		Scene			* currentScene		= _appModel->getCurrentScene();
		Sequence		* currentSequence	= currentScene->getCurrentSequence();

		if (_vidController->checkState(kVIDCONTROLLER_NEXTVIDERROR) && _switchToSequence != NULL) {
			LOG_VERBOSE("ERROR on load. Try again? This is super-inelegant but does seem to let us keep running");
			_vidController->loadMovie(_switchToSequence, true);
		}

		if (_vidController->checkState(kVIDCONTROLLER_CURRENTVIDONE)) {
			// the video just finished and we toggled to next video if there is one

			if(currentScene->nextSequence()) {
				// loaded next sequence in this scene, keep going
				LOG_VERBOSE("Gone to next sequence");
				_vidController->setState(kVIDCONTROLLER_READY);
			} else {
                nextScene();
			}

		}

		// catch _switchToSequence when a movie is loaded completely
		if (_switchToSequence != NULL && _vidController->checkState(kVIDCONTROLLER_NEXTVIDREADY)) {
		     _lastActionTime = ofGetElapsedTimeMillis();
			_vidController->toggleVideoPlayers();
			_vidController->update();
			_vidController->setState(kVIDCONTROLLER_READY);
			currentScene->setCurrentSequence(_switchToSequence);
			_switchToSequence = NULL;
		}

		// AUTO TESTING USER INTERACTION CODE
		if (ofGetElapsedTimeMillis() - _lastAutoActionTime > 20000 && boost::any_cast<bool>(_appModel->getProperty("autoTest"))) {
			_lastAutoActionTime = ofGetElapsedTimeMillis();
			int opt = (int)ofRandom(0, 10);
			switch (opt) {
				case 0:
					LOG_VERBOSE("AUTO ACTION: kVictimAction");
					_appModel->setProperty("userAction", kVictimAction);
					break;
				case 1:
					LOG_VERBOSE("AUTO ACTION: Fast-forward movie (13 frames from end)");
					_appModel->getCurrentVideoPlayer()->setFrame(_appModel->getCurrentFrameTotal()-13);
					break;
				default:
					LOG_VERBOSE("AUTO ACTION: kAttackerAction");
					_appModel->setProperty("userAction", kAttackerAction);
					break;
			}

		}

        _camControllers[0]->update();
		_camControllers[1]->update();

		_vidController->update();

	}

}

void AppController::nextScene() {
    LOG_VERBOSE("Current scene ended, rewind current scene to first sequence. Loading next scene.");

    Scene			* currentScene		= _appModel->getCurrentScene();
    // rewind last scene
    currentScene->rewindScene();
    // load next scene
    _appModel->nextScene();
    currentScene = _appModel->getCurrentScene();
    _switchToSequence = currentScene->getCurrentSequence();
    _vidController->loadMovie(_switchToSequence, true);
}

//--------------------------------------------------------------
void AppController::draw() {
	//	LOG_VERBOSE("Drawing");
	ofSetColor(255, 255, 255, 255);
	_appView->draw();
	glPushMatrix();
	glTranslatef(0.0f, 40.0f, 0.0f);
	_camControllers[0]->_greyImage.draw(0,0);
	for (int face = 0; face < _camControllers[0]->_finder.blobs.size(); face++) {
	    ofSetColor(255,0,0);
	    ofNoFill();
	    ofRect(_camControllers[0]->_finder.blobs[face].boundingRect.x,
               _camControllers[0]->_finder.blobs[face].boundingRect.y,
               _camControllers[0]->_finder.blobs[face].boundingRect.width,
               _camControllers[0]->_finder.blobs[face].boundingRect.height);

	}
	ofSetColor(255,255,255);
	glTranslatef(1920.0f/5.0f, 0.0f, 0.0f);
	_camControllers[1]->_greyImage.draw(0,0);
    for (int face = 0; face < _camControllers[1]->_finder.blobs.size(); face++) {
	    ofSetColor(255,0,0);
	    ofNoFill();
	    ofRect(_camControllers[1]->_finder.blobs[face].boundingRect.x,
               _camControllers[1]->_finder.blobs[face].boundingRect.y,
               _camControllers[1]->_finder.blobs[face].boundingRect.width,
               _camControllers[1]->_finder.blobs[face].boundingRect.height);

	}
	ofSetColor(255,255,255);
}

//--------------------------------------------------------------
void AppController::keyPressed(int key){

	float gamma                 = boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection"));
	float blend                 = boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio"));
	bool showUnmask             = boost::any_cast<bool>(_appModel->getProperty("showUnmaskedTextures"));
	bool autoTest               = boost::any_cast<bool>(_appModel->getProperty("autoTest"));
    bool showProps              = boost::any_cast<bool>(_appModel->getProperty("showProps"));
    bool showFFT                = boost::any_cast<bool>(_appModel->getProperty("showFFT"));
    bool showDebug              = boost::any_cast<bool>(_appModel->getProperty("showDebugView"));
    int scaleMethod             = boost::any_cast<int>(_appModel->getProperty("tryScaleMethod"));
    string cameraToAdjust       = boost::any_cast<string>(_appModel->getProperty("cameraToAdjust"));
    string cameraPropToAdjust   = boost::any_cast<string>(_appModel->getProperty("cameraPropToAdjust"));
    float camRotation           = boost::any_cast<float>(_appModel->getProperty("camRotation"+cameraToAdjust));
    float camScale              = boost::any_cast<float>(_appModel->getProperty("camScale"+cameraToAdjust));
    float camPositionX          = boost::any_cast<float>(_appModel->getProperty("camPositionX"+cameraToAdjust));
    float camPositionY          = boost::any_cast<float>(_appModel->getProperty("camPositionY"+cameraToAdjust));

    float rotationAdjustment    = 0.5f;
    float scaleAdjustment       = 0.01f;
    float positionAdjustment    = 5.0f;

    float fakeInput             = 1024.0f; // this could be faked more realistically using time between keys etc -> will do for now

	switch (key) {
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
			break;
        case 'v':
			_camControllers[0]->loadSettings();
			_camControllers[1]->loadSettings();
			break;
#endif
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
		case 't':
			//_appModel->setProperty("autoTest", !autoTest);
			break;
		case 'q':
			_micController->fakeVictimAction(fakeInput*8.0f);
			break;
		case 'p':
			_ardController->fakeAttackAction(fakeInput);
			break;
		case 'S':
			_dataController->saveProperties();
			break;
		case ' ':
            FaceEvent(fakeInput);
			break;
        case 'm':
            _appModel->getCurrentVideoPlayer()->togglePaused();
            break;
        case '/':
            _appModel->getCurrentVideoPlayer()->setFrame(_appModel->getCurrentFrameTotal()-13);
            break;
		case '.':
            _appModel->getCurrentVideoPlayer()->nextFrame();
            break;
		case ',':
            _appModel->getCurrentVideoPlayer()->previousFrame();
			break;
        case '1':
            _appModel->setProperty("cameraToAdjust", (string)"0");
            break;
        case '2':
            _appModel->setProperty("cameraToAdjust", (string)"1");
            break;
        case '5':
            _appModel->setProperty("cameraPropToAdjust", (string)"POSITION");
            break;
        case '6':
            _appModel->setProperty("cameraPropToAdjust", (string)"SCALEROTATION");
            break;
		case 356: // left arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                camRotation -= rotationAdjustment;

            }
            if (cameraPropToAdjust == "POSITION") {
                camPositionX -= positionAdjustment;
            }
			break;
		case 358: // right arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                camRotation += rotationAdjustment;
            }
            if (cameraPropToAdjust == "POSITION") {
                camPositionX += positionAdjustment;
            }
			break;
        case 359: // up arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                camScale -= scaleAdjustment;
            }
            if (cameraPropToAdjust == "POSITION") {
                camPositionY -= positionAdjustment;
            }
			break;
		case 357: // down arrow
            if (cameraPropToAdjust == "SCALEROTATION") {
                camScale += scaleAdjustment;
            }
            if (cameraPropToAdjust == "POSITION") {
                camPositionY += positionAdjustment;
            }
			break;
        case 'r':
            camPositionX = 0.0f;
            camPositionY = 0.0f;
            camScale = 0.5f;
            camRotation = 0.0f;
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
             _camControllers[0]->_doFaceDetection ^= true;
             _camControllers[1]->_doFaceDetection ^= true;
            break;
		default:
			break;
	}

	_appModel->setProperty("shaderBlendRatio", blend);
	_appModel->setProperty("shaderGammaCorrection", gamma);

    _appModel->setProperty("camRotation"+cameraToAdjust, camRotation);
    _appModel->setProperty("camScale"+cameraToAdjust, camScale);
    _appModel->setProperty("camPositionX"+cameraToAdjust, camPositionX);
    _appModel->setProperty("camPositionY"+cameraToAdjust, camPositionY);

    PosRotScale prs;

    prs.x           = camPositionX;
    prs.y           = camPositionY;
    prs.r           = camRotation;
    prs.s           = camScale;

    LOG_NOTICE(cameraPropToAdjust + " on camera " + cameraToAdjust);

    if (cameraToAdjust == "0") {
        _camControllers[0]->setCameraAttributes(prs); // could do this direct on model but seems more apt to go cia the cam controllers
    } else {
        _camControllers[1]->setCameraAttributes(prs); // could do this direct on model but seems more apt to go cia the cam controllers
    }


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
    if (!boost::any_cast<bool>(_appModel->getProperty("fullScreen")))
    {
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
    }
    else
    {
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

