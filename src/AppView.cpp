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
	_attackView     = new AttackView(146, 1080);
	_victimView     = new VictimView(146, 1080);

}

AppView::~AppView() {

    LOG_NOTICE("Destroying all other (sub) views");
#if OF_VERSION < 7
    _viewFBO.detach();
    _viewFBOTexture.clear();
#endif

    delete _loadingView;
    delete _sceneView;
    delete _debugView;
    delete _attackView;
    delete _victimView;

}

void AppView::update() {

	ofBackground(0, 0, 0);

	if(_appModel->checkState(kAPP_LOADING)){
		_loadingView->update();
	} else {
		_sceneView->update();
		_attackView->update();
		_victimView->update();
		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))) {
			_debugView->update();
		}
	}
}

void AppView::draw() {

    ofEnableAlphaBlending();

    // fullscreen shenanigans
    //bool isFullScreen = boost::any_cast<bool>(_appModel->getProperty("fullScreen"));
#ifdef EXTENDED_DISPLAY
    float width = CLAMP((float)ofGetWidth(), 0.0f , 1920.0f);
#else
    float width = (float)ofGetWidth();
#endif

    float height = (float)ofGetHeight();//(float)ofGetHeight();

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
#ifdef EXTENDED_DISPLAY
        if (ofGetWidth() > 1920) _sceneView->draw(width, 0, width, height);
#endif
        if (!_appModel->checkCurrentInteractivity(kINTERACTION_FACE)) {
#ifdef EXTENDED_DISPLAY
            _attackView->draw(1920 + (1920 - _attackView->getWidth()) * (width/1920.0f), 0, _attackView->getWidth() * (width/1920.0f), _attackView->getHeight() * (height/1080.0f));
#else
            _attackView->draw((1920 - _attackView->getWidth()) * (width/1920.0f), 0, _attackView->getWidth() * (width/1920.0f), _attackView->getHeight() * (height/1080.0f));
#endif
            _victimView->draw(0, 0, _victimView->getWidth() * (width/1920.0f), _victimView->getHeight() * (height/1080.0f));
        }


		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))){
#if OF_VERSION < 7
			_debugView->draw(0, 0, width, height);
#else
            _debugView->draw(0, 0, width, height);
#endif
		}
	}

    ofDisableAlphaBlending();

}
