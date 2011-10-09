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
    void            FaceEvent(int & level);

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
	int             _lastActionTime;
    int             _lastFrameWhenForced;
    int             _returnFrameWhenForced;

protected:

//#define RUN_IN_TEST_MODE 1
//#ifdef RUN_IN_TEST_MODE
//	/* Test functions */
//public:
//	#define kTOTAL_TEST_EVENT_TYPES (6)
//	typedef enum rktEventType {
//		face1ShowEvent,
//		face2ShowEvent,
//		face1HideEvent,
//		face2HideEvent,
//		buttonEvent,
//		micEvent
//	} rktEventType;
//	
//	bool	_facesPresent[2];
//	int		_timeLastEvent[kTOTAL_TEST_EVENT_TYPES];
//	int		_lastTimeAnyEvent;
//	
//	void testPreflight(){
//		_facesPresent[0] = false;
//		_facesPresent[1] = false;
//		for(int i = 0; i < kTOTAL_TEST_EVENT_TYPES; i++){
//			_timeLastEvent[i] = -1;
//		}
//		_lastTimeAnyEvent = -1;
//	};
//	
//	int getRandomEventType(){
//		return (int)(ofRandom(kTOTAL_TEST_EVENT_TYPES));
//	};
//	
//	void updateTester(){
//		if(ofGetElapsedTimeMillis() > 5000){
//			fireEvent(buttonEvent);
//		}			
//	};
//	//bool TestController::fireEvent(rktEventType event){
//	//	_timeLastEvent[event] = ofGetElapsedTimeMillis();
//	//	if(event == face1ShowEvent){
//	//		// trigger
//		ofNotify(apc->_camController->cameraEvent, apc->_camController, level);//
//		return true;
//	}
//	if(event == face1HideEvent){
//		return true;
//	}
//	if(event == face2ShowEvent){
//
//		return true;
//	}
//	if(event == face2HideEvent){
//		
//		return true;
//	}
//	if(event == buttonEvent){
//		float area = ofRandom(10.0); // TODO: What to use here?
//			ofNotifyEvent(_appController->_ardController->attackAction, area, _appController->_ardController);
//			return true;
//		}
//		if(event == micEvent){
//			
//			return true;
//	}
//	return false;
//	
//};	
//private:	
//#endif
};

#endif
