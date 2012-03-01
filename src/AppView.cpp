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

    _fade = 1.0f;
    _fadeUp = false;

	_loadingView    = new LoadingView(width, height);
	_sceneView      = new SceneView(width, height);
#ifdef DEBUG_VIEW_ENABLED
	_debugView      = new DebugView(width, height);
#endif
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
#ifdef DEBUG_VIEW_ENABLED
    delete _debugView;
#endif
    delete _attackView;
    delete _victimView;

}

void AppView::update() {

	ofBackground(0, 0, 0);

     if (!_fadeUp) _fade = 1.0f;

	if(_appModel->checkState(kAPP_LOADING)){
		_loadingView->update();
	} else {
		_sceneView->update();
		_attackView->update();
		_victimView->update();
#ifdef DEBUG_VIEW_ENABLED
		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))) {
			_debugView->update();
		}
#endif
	}

	Scene * currentScene = _appModel->getCurrentScene();
	if (currentScene == NULL) return;

	Sequence * currentSequence = _appModel->getCurrentSequence();
	if (currentSequence == NULL) return;

    int frame = _appModel->getCurrentSequenceFrame();
    int total  = _appModel->getCurrentSequenceNumFrames();

	if (currentSequence->getNumber() == 8) {
        if(frame > total - 24) { // start fade 2 secs from end
            _fade = 1.0 * (total-frame)/24.0f;
            _fadeUp = true;
        }
    }

    if (_fadeUp) {
        if (frame < 12) _fade = 1.0 * (float)frame/12.0f;
        if (_fade >= 0.9) _fadeUp = false;
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
		// glPopMatrix(); // un-needed? ollie 11/10/11
	} else {

		// composite all views
        glColor3f(_fade, _fade, _fade);

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

#ifdef DEBUG_VIEW_ENABLED
        glColor3f(1.0f, 1.0f, 1.0f);
		if(boost::any_cast<bool>(_appModel->getProperty("showDebugView"))){
			_debugView->draw();
        }
#endif
	}

    ofDisableAlphaBlending();

}
