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
AppController::AppController() {
	//nothing for now (NB: I put the Logger instantiation in main.h)
}

//--------------------------------------------------------------
AppController::~AppController() {
	// nothing for now but we should clean up here
}

//--------------------------------------------------------------
void AppController::setup() {
	
	LOG_NOTICE("Initialising");
	
	ofSetVerticalSync(true);
	
	_isFullScreen		= false; // change this when we start in fullscreen mode
	_switchToSequence	= NULL;
	
	_appModel->registerStates();
	_appModel->setState(kAPP_INIT);
	
	// set up datacontroller
	_dataController = new DataController(ofToDataPath("config_properties.xml"));
	_dataController->registerStates();
	
	// setup videoController
	_vidController = new VideoController();
	_vidController->registerStates();
	
	// setup micController
	_micController = new MicController();
	_micController->registerStates();
	
	// setup ardController
	_ardController = new ArdController();
	_ardController->registerStates();
	
	// setup cameras
	_camControllers[0] = new CamController();
	_camControllers[1] = new CamController();
	
	_camControllers[0]->setup("Built-in iSight", 640, 480);
	_camControllers[1]->setup("ManyCam Virtual Webcam (RGB)", 640, 480);	// NB: had to use QTKit to get ManyCam working
	
	// register pointers to textures from cams on the model
	_appModel->setCameraTextures(_camControllers[0]->getCamTextureRef(), _camControllers[1]->getCamTextureRef());
	
	// set default for user action, temporary way to handle this stuff!
	_appModel->setProperty("userAction", kNoUserAction);
	
	// setup main app view
	_appView = new AppView(boost::any_cast<float>(_appModel->getProperty("appViewWidth")),
						   boost::any_cast<float>(_appModel->getProperty("appViewHeight")));
	
	
	_appModel->setState(kAPP_LOADING);
	
	// set app state;
	_appModel->setProperty("loadingMessage", string("AppController loading"));
	_appModel->setProperty("loadingProgress", 0.1f);
	
	// scene parsing properties
	_appModel->setProperty("parseRequiresTransformReanalysis", false);
	_appModel->setProperty("parseRebuildXML", true);
	
	
	LOG_NOTICE("Initialisation complete");
}

void AppController::swapCameras() {
	// use pointer swap and re-register texture references on the model
	swap(_camControllers[0], _camControllers[1]);
	_appModel->setCameraTextures(_camControllers[0]->getCamTextureRef(), _camControllers[1]->getCamTextureRef());
}

//--------------------------------------------------------------
void AppController::update() {
	//	LOG_VERBOSE("Updating");
	
	// if we're loading, update datacontroller
	if(_appModel->checkState(kAPP_LOADING)){
		
		_dataController->update();
		
		if(_dataController->checkState(kDATACONTROLLER_FINISHED)){
			
			// DC finished...let's load the first movie
			Scene			* currentScene;
			goThreadedVideo * movie;
			
			// get current scene
			currentScene = _appModel->getCurrentScene();
			
			// force load using _switchToSequence var which will be caught below when the movie is fully loaded...
			_switchToSequence = currentScene->getCurrentSequence();
			_vidController->loadMovie(_switchToSequence, true);
			_appModel->setState(kAPP_RUNNING);
			
		}
	}
	
	// running, so update scenes, etc
	if(_appModel->checkState(kAPP_RUNNING)) {
		
		Scene			* currentScene		= _appModel->getCurrentScene();
		Sequence		* currentSequence	= currentScene->getCurrentSequence();
		
		_camControllers[0]->update();
		_camControllers[1]->update();
		
		_vidController->update();
		
		if (_vidController->checkState(kVIDCONTROLLER_CURRENTVIDONE)) {
			// the video just finished and we toggled to next video if there is one
			
			if(currentScene->nextSequence()) {
				// loaded next sequence in this scene, keep going
				LOG_VERBOSE("Gone to next sequence");
			} else {
				LOG_WARNING("Current scene ended, rewind current scene to first sequence. Loading next scene.");
				
				// rewind last scene
				currentScene->rewindSequences();
				
				// load next scene
				_appModel->nextScene();
				currentScene = _appModel->getCurrentScene();
				_switchToSequence = currentScene->getCurrentSequence();
				_vidController->loadMovie(_switchToSequence, true);
			}
			
			// re call update on vidcontroller so everything is sweet and seq, scene and movs all match
			_vidController->update();
			_vidController->setState(kVIDCONTROLLER_READY);
		}
		
		// check user actions and que movies and the sequece to _switchTo...
		int userAction = boost::any_cast<int>(_appModel->getProperty("userAction"));
		
		if (currentSequence->getInteractivity() == "both" && _switchToSequence == NULL) {
			
			// Check for interactive event
			// this->hasInteractiveEventFlag()
			// we have had an interactive event
			
			if (userAction == kAttackerAction) {
				LOG_VERBOSE("Interactive action: Attacker");	
				_appModel->setProperty("userAction", kNoUserAction);
				_switchToSequence = currentScene->getSequence(currentSequence->getAttackerResult());
				_vidController->loadMovie(_switchToSequence, true);
				
	
			} else if (userAction == kVictimAction){
				LOG_VERBOSE("Interactive action: Victim");	
				_appModel->setProperty("userAction", kNoUserAction);
				_switchToSequence = currentScene->getSequence(currentSequence->getVictimResult());
				_vidController->loadMovie(_switchToSequence, true);
			}
			
		}
		
		if (currentSequence->getInteractivity() == "victim" && _switchToSequence == NULL) {
			if(userAction == kVictimAction){
				LOG_VERBOSE("Interactive action: Victim");
				_appModel->setProperty("userAction", kNoUserAction);
				_switchToSequence = currentScene->getSequence(currentSequence->getVictimResult());
				_vidController->loadMovie(_switchToSequence, true);
			}
			
		} 
		
		// catch _switchToSequence when a movie is loaded completely
		if (_vidController->checkState(kVIDCONTROLLER_NEXTVIDREADY) && _switchToSequence != NULL) {
			_vidController->toggleVideoPlayers();
			_vidController->update();
			_vidController->setState(kVIDCONTROLLER_READY);
			currentScene->setCurrentSequence(_switchToSequence);
			_switchToSequence = NULL;
		}
		
	}

	_appView->update();
}

//--------------------------------------------------------------
void AppController::draw() {
	//	LOG_VERBOSE("Drawing");
	ofSetColor(255, 255, 255, 255);
	_appView->draw();
}

//--------------------------------------------------------------
void AppController::keyPressed(int key){
	
	float gamma = boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection"));
	float blend = boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio"));
	bool showUnmask = boost::any_cast<bool>(_appModel->getProperty("showUnmaskedTextures"));
	
	switch (key) {
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
			_appModel->setProperty("showDebugView", !boost::any_cast<bool>(_appModel->getProperty("showDebugView")));
			break;
		case 'q':
			_appModel->setProperty("userAction", kVictimAction);
			break;
		case 'p':
			_appModel->setProperty("userAction", kAttackerAction);
			break;
		case 'P':
			_dataController->saveProperties();
			break;
		case 'm':
			swapCameras();
			break;
		case ' ':
			_appModel->getCurrentVideoPlayer()->togglePaused();
			break;
		case '>':
			_appModel->getCurrentVideoPlayer()->setFrame(_appModel->getCurrentFrameTotal()-24);
			break;
		case 356: // left arrow
			_appModel->getCurrentVideoPlayer()->previousFrame();
			break;
		case 358: // right arrow
			_appModel->getCurrentVideoPlayer()->nextFrame();
			break;
		case 'h':
			_appModel->setProperty("showUnmaskedTextures", (showUnmask ? false : true));
			break;
		case 'f':
			toggleFullscreen();
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
    if (!_isFullScreen)
    {
        LOG("Trying to force fullscreen on Windows 7" + ofToString(ofGetWidth()));
        windowTitle = "imMediate";
        ofSetWindowTitle(windowTitle);
        int x = 0;
        int y = 0;
        int width = W_CONTROL_SCREEN + W_OUTPUT_SCREEN;
        int height = H_CONTROL_SCREEN;
        int storedWindowX, storedWindowY, storedWindowH, storedWindowW;
        HWND vWnd  = FindWindow(NULL,  "imMediate");
        long windowStyle = GetWindowLong(vWnd, GWL_STYLE);
        windowStyle &= ~WS_OVERLAPPEDWINDOW;
        windowStyle |= WS_POPUP;
        SetWindowLong(vWnd, GWL_STYLE, windowStyle);
        SetWindowPos(vWnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
        bCustomFullscreen = true;
    }
    else
    {
        int x = 0;
        int y = 0;
        int width = W_CONTROL_SCREEN;
        int height = H_CONTROL_SCREEN;
        HWND vWnd  = FindWindow(NULL,  "imMediate");
        long windowStyle = GetWindowLong(vWnd, GWL_STYLE);
        windowStyle |= WS_TILEDWINDOW;
        SetWindowLong(vWnd, GWL_STYLE, windowStyle);
        SetWindowPos(vWnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
        bCustomFullscreen = false;
    }
	
#else
    ofToggleFullscreen();
#endif
}

