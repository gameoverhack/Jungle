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
	// nothing?
}

void DebugView::update(){
	
	// set pointers for current Scene, Sequence Movie etc
	goVideoPlayer	* currentMovie		= _appModel->getSequenceMovie();
	Sequence		* currentSequence	= _appModel->getCurrentSequence();
	Scene			* currentScene		= _appModel->getCurrentScene();

	// get frame rate
	string msg = "ABC: " + ofToString(ofGetFrameRate()) + "\n";
	
	// get transform info and current movie frame
	int currentFrame	= currentMovie->getCurrentFrame();
	int totalFrames		= currentMovie->getTotalNumFrames();
	
	msg += "vic1 Transform: " + currentSequence->getTransformAsString(0, currentFrame) + "\n";
	msg += "atk1 Transform: " + currentSequence->getTransformAsString(1, currentFrame) + "\n";
	if (currentSequence->getTransformCount() > 2) {
		msg += "atk2 Transform: " + currentSequence->getTransformAsString(2, currentFrame) + "\n";
	}
	
	// get all the properties on the AppModel
	msg += _appModel->getAllPropsAsList();

	_viewFBO.begin();
	glPushMatrix();
	glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
	glClear(GL_COLOR_BUFFER_BIT);
	
	// draw strings
	ofSetColor(0, 255, 0, 255);
	ofDrawBitmapString(msg, 20, 20); // this gets rendered upside down for some reason ?*/
	
	// draw scene progression
	float percent = (float)currentFrame/(float)(totalFrames);
	float width = 300;
	float height = 20;
	ofSetColor(50, 50, 50);
	ofRect(5, _viewHeight-height-10, width+2, height+2);
	ofSetColor(200, 50, 50);
	ofRect(5+1, _viewHeight-height-9, width*percent, height);

	// give us info about progression
	msg = currentScene->getName() + "::" + currentSequence->getName() + "::" + ofToString(currentFrame) + "/" + ofToString(totalFrames);
	
	ofSetColor(255,255,255);
	ofDrawBitmapString(msg, 6, _viewHeight-height-4);

	glPopMatrix();
	_viewFBO.end();
}

void DebugView::draw(){
	glPushMatrix();
	glTranslatef(0.0, _viewHeight, 0.0);
	glScalef(1.0, -1.0, 1.0);
	BaseView::draw();
	glPushMatrix();
}