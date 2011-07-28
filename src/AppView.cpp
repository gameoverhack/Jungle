/*
 *  AppView.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "AppView.h"

AppView::AppView(float width, float height) : BaseView(width ,height) {
	_loadingView = new LoadingView(width, height);
	_sceneView = new SceneView(width, height);
	_debugView = new DebugView(width, height);
}

void AppView::update() {
	
	ofBackground(0, 0, 0);
	
	if(_appModel->checkState(kAPP_LOADING)){
		_loadingView->update();
	} else {
		_sceneView->update();
		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))){
			_debugView->update();
		}
	}
}

void AppView::draw() {
	
	glPushMatrix();
	
	// scale to window or fullscreen size
	float renderWidth = boost::any_cast<float>(_appModel->getProperty("appViewWidth"));
	float renderHeight = boost::any_cast<float>(_appModel->getProperty("appViewHeight"));
	float screenScale = (float)ofGetWidth()/renderWidth;
	float offsetY = ((float)ofGetHeight() - (renderHeight*screenScale))/2.0;
	glTranslatef(0, offsetY, 0);
	glScalef(screenScale, screenScale, 0.0f);
	
	if(_appModel->checkState(kAPP_LOADING)){
		_loadingView->draw();
		glPopMatrix();
	} else {
			
		// composite all views

		
		_sceneView->draw();
		
		glPopMatrix();
		
		// draw Mic view
		// draw smasher view
		// draw diagnositc view
		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))){
			_debugView->draw();
		}
	}
	
}