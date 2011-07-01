/*
 *  AppView.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "AppView.h"

AppView::AppView(int width, int height) : BaseView(width ,height) {
	_sceneView = new SceneView(width, height);
}

void AppView::update() {
	_sceneView->update();
}

void AppView::draw() {
	
	/* composite all views */
	
	_sceneView->draw();
	
	/* draw Mic view */
	/* draw smasher view */
	/* draw diagnositc view */
	
}

