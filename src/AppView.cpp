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

    LOG_NOTICE("Setting up all (sub) views");
	_loadingView    = new LoadingView(width, height);
	_sceneView      = new SceneView(width, height);
	_debugView      = new DebugView(width, height);
	_attackView     = new AttackView(width, height);

}

AppView::~AppView() {

    LOG_NOTICE("Destroying all other (sub) views");

    _viewFBO.detach();
    _viewFBOTexture.clear();

    delete _loadingView;
    delete _sceneView;
    delete _debugView;
    delete _attackView;

}

void AppView::update() {

	ofBackground(0, 0, 0);

	if(_appModel->checkState(kAPP_LOADING)){
		_loadingView->update();
	} else {
		_sceneView->update();
		_attackView->update();
		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))) {
			_debugView->update();
		}
	}
}

void AppView::draw() {

    // fullscreen shenanigans
    //bool isFullScreen = boost::any_cast<bool>(_appModel->getProperty("fullScreen"));
    float width = CLAMP((float)ofGetWidth(), 0.0f , 1920.0f);
    float height = (float)ofGetHeight();

	if(_appModel->checkState(kAPP_LOADING)){
		_loadingView->draw(0, height, width, -height);
		glPopMatrix();
	} else {

		// composite all views

		_sceneView->draw(0, 0, width, height);
		// TODO: this dual screen draw is tearing on Windows...and presumably on Mac OSX in GLUT...
		// options for solving include: use OSX and ofxCocoa or look into multiple openGL contexts and
		// shared textures and/or switched context drawing...
		// ...see http://forum.openframeworks.cc/index.php/topic,4872.0.html
        //if (isFullScreen && ofGetWidth() > 1920) _sceneView->draw(width, 0, width, height);

		// draw Mic view
		// draw smasher view
		// draw diagnositc view
		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))){
			_debugView->draw(0, height, width, -height);
			_attackView->draw(0, height, width, -height);
		}
	}

}
