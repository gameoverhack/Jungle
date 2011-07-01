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
	ofSetColor(0, 255, 0, 255);
	ofDrawBitmapString(msg, 20, 20); /* this gets rendered upside down for some reason ?*/
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