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
	/* nothing? */
}

void DebugView::update(){
	string msg = "ABC: " + ofToString(ofGetFrameRate()) + "\n";
	msg += _appModel->getAllPropsAsList();

	_viewFBO.begin();
	glPushMatrix();
	glClearColor(0.0, 0.0, 0.0, 0.0); /* transparent clear colour */
	glClear(GL_COLOR_BUFFER_BIT);
	
	/* draw strings */
	ofSetColor(0, 255, 0, 255);
	ofDrawBitmapString(msg, 20, 20); /* this gets rendered upside down for some reason ?*/
	
	/* draw scene progression */
	goVideoPlayer *video = _appModel->getCurrentSequence()->getSequenceMovie();
	Sequence * seq = _appModel->getCurrentSequence();
	Scene * scn = _appModel->getCurrentScene();
	
	float percent = video->getCurrentFrame()/(float)(video->getTotalNumFrames());
	float width = 300;
	float height = 20;
	ofSetColor(50, 50, 50);
	ofRect(5, _viewHeight-height-10, width+2, height+2);
	ofSetColor(200, 50, 50);
	ofRect(5+1, _viewHeight-height-9, width*percent, height);

	msg = scn->getName() + "::" + seq->getName();
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