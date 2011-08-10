/*
 *  LoadingView.cpp
 *  Jungle
 *
 *  Created by ollie on 15/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "LoadingView.h"

LoadingView::LoadingView(float width, float height) : BaseView(width ,height) {

}

void LoadingView::update() {
	float progress = boost::any_cast<float>(_appModel->getProperty("loadingProgress"));
	string message = boost::any_cast<string>(_appModel->getProperty("loadingMessage"));


	_viewFBO.begin();
	glPushMatrix();
    glTranslatef(0.0, _viewHeight, 0.0);
	glScalef(1.0, -1.0, 1.0);

	glClearColor(0.1, 0.1, 0.1, 1.0); // transparent clear colour
	glClear(GL_COLOR_BUFFER_BIT);

	float progressionWidth = _viewWidth-60;
	float progressionHeight = 50;
	float x = 30;
	float y = (_viewHeight/2.0)-(progressionHeight/2.0);

	ofSetColor(50, 50, 50, 220);
	ofRect(x, y, progressionWidth, progressionHeight);

	ofSetColor(200, 50, 50, 220);
	ofRect(x, y+1, (progressionWidth-2)*progress, progressionHeight-2);


	ofSetColor(0,255,0);
	ofDrawBitmapString(message, x, y+progressionHeight+2);

	glPopMatrix();
	_viewFBO.end();

}

/*void LoadingView::draw(){
	glPushMatrix();
	glTranslatef(0.0, _viewHeight, 0.0);
	glScalef(1.0, -1.0, 1.0);
	BaseView::draw();
	glPushMatrix();
}*/
