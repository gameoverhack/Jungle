/*
 *  DebugView.cpp
 *  Jungle
 *
 *  Created by ollie on 30/06/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DebugView.h"


DebugView::DebugView(float width, float height) : BaseView(width, height){
	LOG_NOTICE("Setting up DebugView");
}

void DebugView::update(){

	// set pointers for current Scene, Sequence Movie etc
	//goThreadedVideo	* currentMovie		= _appModel->get();
	Sequence		* currentSequence	= _appModel->getCurrentSequence();
	Scene			* currentScene		= _appModel->getCurrentScene();
    int             * ardRawPins = _appModel->getARDRawPins();
    string          msg;

    if(ardRawPins != NULL) {

        msg += "ard RAW pin0: " + ofToString(ardRawPins[0]) + "\n";
        msg += "ard RAW pin1: " + ofToString(ardRawPins[1]) + "\n";

    }

	// get frame rate
	msg += "ABC: " + ofToString(ofGetFrameRate()) + "\n";

	// get transform info and current movie frame
	int currentFrame	= _appModel->getCurrentFrame(); //currentMovie->getCurrentFrame();
	int totalFrames		= _appModel->getCurrentFrameTotal(); //currentMovie->getTotalNumFrames();

	//currentFrame = CLAMP(currentFrame, 0, totalFrames-1); // why are you so cruel?
    if (boost::any_cast<bool>(_appModel->getProperty("showProps"))) {
        msg += "vic1 Transform: " + currentSequence->getTransformAsString("vic1", currentFrame) + "\n";
        msg += "atk1 Transform: " + currentSequence->getTransformAsString("atk1", currentFrame) + "\n";
        if (currentSequence->getTransformCount() > 2) {
            msg += "atk2 Transform: " + currentSequence->getTransformAsString("atk2", currentFrame) + "\n";
        }

        // get all the properties on the AppModel
        msg += _appModel->getAllPropsAsList();
    }

	_viewFBO.begin();
	glPushMatrix();

	glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
	glClear(GL_COLOR_BUFFER_BIT);

	// draw strings
	ofSetColor(0, 255, 0, 255);
	ofDrawBitmapString(msg, 20, 20); // this gets rendered upside down for some reason ?*/

	// draw scene progression
	float progressionPercentage = (float)currentFrame/(float)(totalFrames);
	float progressionWidth = 300;
	float progressionHeight = 20;

	ofSetColor(50, 50, 50, 220);
	ofRect(5, _viewHeight-progressionHeight-10, progressionWidth+4, progressionHeight+2);

	// valid movie file/sequence stuff
	if(currentSequence->getIsSequenceFaked()) {
		ofSetColor(255, 50, 50, 220);
	}
	else if(currentSequence->getIsMovieFaked()) {
		ofSetColor(255, 120, 0, 220);
	} else {
		ofSetColor(50, 220, 50, 220);
	}

	ofRect(5+1, _viewHeight-progressionHeight-9, progressionWidth*progressionPercentage, progressionHeight);

	// give us info about progression
	msg = currentScene->getName() + "::" + currentSequence->getName() + "::" + ofToString(currentFrame) + "/" + ofToString(totalFrames);

	ofSetColor(255,255,255);
	ofDrawBitmapString(msg, 6, _viewHeight-progressionHeight-4);

	glPopMatrix();
	_viewFBO.end();
}

/*void DebugView::draw(){
	glPushMatrix();
	glTranslatef(0.0, ofGetHeight(), 0.0);
	glScalef(1.0, -1.0, 1.0);
	BaseView::draw();
	glPushMatrix();
}*/
