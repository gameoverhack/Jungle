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
	_sceneView = new SceneView(width, height);
	_debugView = new DebugView(width, height);
}

void AppView::update() {
	_sceneView->update();
	_debugView->update();
}

void AppView::draw() {
	
	/* composite all views */
	
	_sceneView->draw();
	
	/* draw Mic view */
	/* draw smasher view */
	/* draw diagnositc view */
	if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))){
		_debugView->draw();
	}
}

