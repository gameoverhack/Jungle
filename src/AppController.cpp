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
	LOGGER->setLogLevel(JU_LOG_VERBOSE);
	LOG_NOTICE("Initialising");
	DataController dataController(ofToDataPath("config.xml"));

	cout << _appModel->getAllPropsAsList() << endl;
	
	_appModel->setProperty("userAction", kNoUserAction);
	
	_appView = new AppView(1280, 720);
	LOG_NOTICE("Initialisation complete");
	
}

//--------------------------------------------------------------
void AppController::update() {
//	LOG_VERBOSE("Updating");
	Scene * currentScene;
	Sequence * currentSequence;
	goVideoPlayer * movie;
	
	/* get current scene */
	currentScene = _appModel->getCurrentScene();	
	/* get current sequence */
	currentSequence = currentScene->getCurrentSequence();

	/* check if sequence was interactive */
	if(currentSequence->getIsInteractive()){
		/* Check for interactive event */
		// this->hasInteractiveEventFlag()
		/* we have had an interactive event */
		int userAction = boost::any_cast<int>(_appModel->getProperty("userAction"));
		if( userAction == kAttackerAction){
			LOG_VERBOSE("Interactive action: Attacker");	
			_appModel->setProperty("userAction", kNoUserAction);
			currentScene->setCurrentSequence(currentSequence->getAttackerResult());
		}
		else if(userAction == kVictimAction){
			LOG_VERBOSE("Interactive action: Victim");	
			_appModel->setProperty("userAction", kNoUserAction);
			currentScene->setCurrentSequence(currentSequence->getVictimResult());
		}

		/* else continue playing this video */
		movie = currentSequence->getSequenceMovie();
	}
	else{
		/* Not interactive movie */
		/* check if we're at the ened of the movie */
		movie = currentSequence->getSequenceMovie();
		if(movie->getIsMovieDone()){
			/* at end of non interactive movie, change to next sequence */
			if(currentScene->nextSequence()){
				/* loaded next sequence in this scene, keep going */
			}
			else{
				/* couldn't load next sequence, there isn't one etc */
				/* set this scenes sequence to first sequence (for when we get back to it) */
// TODO:	This needs a method, can just use the key order since our keys are alpha ordered, 
//			i feel odd about doing that though. I guess it is guarenteed though.
//				currentScene->setCurrentSequence(0); 
				
				/* load next scene */
				_appModel->nextScene();
				currentScene = _appModel->getCurrentScene();
				currentSequence = currentScene->getCurrentSequence();
			}
		}
		/* have to call this incase it changed */
		movie = currentSequence->getSequenceMovie();
	}
	
	/* update the movie */
	movie->update();

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
		default:
			break;
	}
	printf("Gamma: %f\n", gamma);
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

