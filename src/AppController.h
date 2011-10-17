/*
 *  AppController.h
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_APPCONTROLLER
#define _H_APPCONTROLLER

#include "ofMain.h"
#include "AppView.h"
#include "AppModel.h"

#include "BaseState.h"
#include "Constants.h"
#include "DataController.h"
#include "CamController.h"
#include "VideoController.h"
#include "MicController.h"
#include "ArdController.h"
#include "SoundController.h"
#include "Analyzer.h"

#include "ofAppGlutWindow.h"

class AppController : public BaseState, public ofBaseApp {
public:

	AppController(ofAppGlutWindow * windowPtr);
	~AppController();

	void setup();
	void update();
	void draw();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	void swapCameras();

	void setWindowed();
	void setFullscreen();
	void toggleFullscreen();

	/* Required for testing */

	ofEvent<float>	updateEvent;

	DataController * getDataController(){
		return _dataController;
	};

	CamController * getCamController(int i){
		return _camControllers[i];
	};

	VideoController * getVideoController() {
		return _vidController;
	};

	MicController * getMicController() {
		return _micController;
	};

	ArdController * getArdController(){
		return _ardController;
	}

	/* End testing */

private:

    void            VictimEvent(float & level);
    void            AttackEvent(float & level);

    void              nextScene();

	// Views
	AppView			* _appView;

	// Controllers
	DataController	* _dataController;
	CamController	* _camControllers[2];
	VideoController * _vidController;
	MicController	* _micController;
	ArdController	* _ardController;
    SoundController * _soundController;

    ofAppGlutWindow * _windowPtr;
	bool			  _isFullScreen;
#ifdef TARGET_WIN32
    char            * _windowTitle;
#endif

	Sequence		* _switchToSequence;

	int				_lastAutoActionTime;
    int             _lastFrameWhenForced;
    int             _returnFrameWhenForced;
    int             _lastAutoAttackAction;
    int             _lastAutoFaceAction;

protected:

};

#endif
