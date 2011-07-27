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
void AppController::setup() {
	
	// set up logger (should be first thing.)
	LOGGER->setLogLevel(JU_LOG_VERBOSE);
	
	LOG_NOTICE("Initialising");
	
	// initial app state
	_state = kAPPCONTROLLER_INIT;
	
	// set up datacontroller
	_dataController = new DataController(ofToDataPath("config_properties.xml"));
	
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
	
	
	_state = kAPPCONTROLLER_LOADING;
	// set app state;
	_appModel->setProperty("appState", (int)_state); // Generally (always should be?) set to AppController's _state
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
	if(_state == kAPPCONTROLLER_LOADING){
		_dataController->update();
		if(_dataController->getState() == kDATACONTROLLER_FINISHED){
			
			// dc finished load first movie
			Scene			* currentScene;
			Sequence		* currentSequence;
			goThreadedVideo * movie;
			
			// get current scene
			currentScene = _appModel->getCurrentScene();	
			
			// get current sequence
			currentSequence = currentScene->getCurrentSequence();

			if (currentSequence->getMovie()->getCurrentlyPlaying() == "") {
				currentSequence->loadMovie();
			} 
			
			// (re)get current movie
			movie = currentSequence->getMovie();
			
			if (!movie->isLoaded()) {
				movie->psuedoUpdate();
				movie->psuedoDraw();
			} else {
				currentSequence->prepareMovie();
				// first movie loaded, update app controller state;
				_state = kAPPCONTROLLER_RUNNING;
				_appModel->setProperty("appState", (int)kAPPCONTROLLER_RUNNING);	// this seems clunky???
			}
			
		}
	}
	
	// running, so update scenes, etc
	if(_state == kAPPCONTROLLER_RUNNING){
		
		_camControllers[0]->update();
		_camControllers[1]->update();
		
		Scene			* currentScene;
		Sequence		* currentSequence;
		Sequence		* loopSequence;
		Sequence		* attackSequence;
		Sequence		* victimSequence;
		goThreadedVideo * currentMovie;
		goThreadedVideo * loopMovie;
		goThreadedVideo * attackerMovie;
		goThreadedVideo * victimMovie;
		
		// get current scene
		currentScene = _appModel->getCurrentScene();	
		
		//currentScene->print();
		
		// get current sequence
		currentSequence = currentScene->getCurrentSequence();
		
		// get current movie refs
		currentMovie	= currentSequence->getMovie();
		
		//cout << currentSequence->getAttackerResult() << " " << currentSequence->getVictimResult() << endl;
		
		if (currentSequence->getNextSequenceName() != "" && currentSequence->getNextSequenceName() != "__FINAL_SEQUENCE__") {
			//cout << "loopCheckSeq" << endl;
			loopSequence	= currentScene->getSequence(currentSequence->getNextSequenceName());
			loopMovie		= loopSequence->getMovie();
			
			if (loopMovie->getCurrentlyPlaying() == "") {
				cout << "loopLoad" << endl;
				loopSequence->loadMovie();
			}
			/*if (!loopMovie->isLoaded()) {
				loopMovie->psuedoUpdate();
				loopMovie->psuedoDraw();
			}*/
		}
		
		if (currentSequence->getAttackerResult() != "" && currentSequence->getAttackerResult() != "__FINAL_SEQUENCE__") {
			cout << "attkCheckSeq" << endl;
			attackSequence	= currentScene->getSequence(currentSequence->getAttackerResult());
			attackerMovie	= attackSequence->getMovie();
			cout << attackerMovie << " " << attackerMovie->isLoading() << endl;
			if (attackerMovie->getCurrentlyPlaying() == "") {
				cout << "attackLoad" << endl;
				attackSequence->loadMovie();
			}
			/*if (!attackerMovie->isLoaded()) {
				attackerMovie->psuedoUpdate();
				attackerMovie->psuedoDraw();
			}*/
		}
		
		if (currentSequence->getVictimResult() != "" && currentSequence->getVictimResult() != "__FINAL_SEQUENCE__") {
			cout << "vikkCheckSeq" << endl;
			victimSequence	= currentScene->getSequence(currentSequence->getVictimResult());
			victimMovie		= victimSequence->getMovie();
			
			if (victimMovie->getCurrentlyPlaying() == "") {
				cout << "victimLoad" << endl;
				victimSequence->loadMovie();
			}
			/*if (!victimSequence->isLoaded()) {
				victimSequence->psuedoUpdate();
				victimSequence->psuedoDraw();
			}*/
		}
		
		
		/*if(currentMovie->getLoopState() ==  && currentSequence->getInteractivity() == "both") {
			currentMovie->setLoopState(OF_LOOP_NORMAL);
		} else {
			currentMovie->setLoopState(OF_LOOP_NONE);
		}*/
		
		// check if sequence was interactive
		if (currentSequence->getInteractivity() == "both") {

			// Check for interactive event
			// this->hasInteractiveEventFlag()
			// we have had an interactive event
			int userAction = boost::any_cast<int>(_appModel->getProperty("userAction"));
			if (userAction == kAttackerAction){
				LOG_VERBOSE("Interactive action: Attacker");	
				_appModel->setProperty("userAction", kNoUserAction);
				currentScene->setCurrentSequence(currentSequence->getAttackerResult());
				//attackerMovie->setPosition(0);
				attackerMovie->psuedoUpdate();
				attackerMovie->psuedoDraw();
				currentSequence->prepareMovie();
			} else if (userAction == kVictimAction){
				LOG_VERBOSE("Interactive action: Victim");	
				_appModel->setProperty("userAction", kNoUserAction);
				currentScene->setCurrentSequence(currentSequence->getVictimResult());
				//victimMovie->setPosition(0);
				victimMovie->psuedoUpdate();
				victimMovie->psuedoDraw();
				currentSequence->prepareMovie();
			}
		}
		
		if (currentSequence->getInteractivity() == "victim") {
			
			int userAction = boost::any_cast<int>(_appModel->getProperty("userAction"));
			if(userAction == kVictimAction){
				LOG_VERBOSE("Interactive action: Victim");
				_appModel->setProperty("userAction", kNoUserAction);
				// TODO Have to check how this works, but for now we'll try to get the next loop, then the victim result of that
				Sequence *loopseq = currentScene->getSequence(currentSequence->getNextSequenceName());
				currentScene->setCurrentSequence(loopseq->getVictimResult());
				//victimMovie->setPosition(0);
				victimMovie->psuedoUpdate();
				victimMovie->psuedoDraw();
				currentSequence->prepareMovie();
			}
		} 
		
		
		// TODO: we just hacked this back to working order!!!
		
		// Not interactive movie
		if(currentMovie->getIsMovieDone()){
			LOG_VERBOSE("It is done, Luke");
			/*loopMovie->setPosition(0);
			*/
			// at end of non interactive movie, change to next sequence
			if(currentScene->nextSequence()) {
				loopMovie->psuedoUpdate();
				loopMovie->psuedoDraw();
				loopSequence->prepareMovie();
				// loaded next sequence in this scene, keep going
				LOG_VERBOSE("Should have gone to next sequence");
			} else {
				// couldn't load next sequence, there isn't one etc
				// set this scenes sequence to first sequence (for when we get back to it)
				// TODO:	This needs a method, can just use the key order since our keys are alpha ordered, 
				//			i feel odd about doing that though. I guess it is guarenteed though.
				//				currentScene->setCurrentSequence(0); 
				LOG_WARNING("Current scene ended, but no method to rewind current scene to first sequence. Loading next scene anyway");
				// load next scene
				_appModel->nextScene();
				currentScene = _appModel->getCurrentScene();
				currentSequence = currentScene->getCurrentSequence();
			}
		}
		
		// have to call this incase it changed
		currentMovie = currentSequence->getMovie();
		
		
		// update the movie
		currentMovie->update();
		
	}
	
	// always update the view
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
			_appModel->getSequenceMovie()->togglePaused();
			break;
		case '>':
			_appModel->getSequenceMovie()->setFrame(_appModel->getSequenceMovie()->getTotalNumFrames()-24);
			break;
		case 356: // left arrow
			_appModel->getSequenceMovie()->previousFrame();
			break;
		case 358: // right arrow
			_appModel->getSequenceMovie()->nextFrame();
			break;
		case 'h':
			_appModel->setProperty("showUnmaskedTextures", (showUnmask ? false : true));
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
	
}

