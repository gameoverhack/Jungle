/*
 *  TestController.cpp
 *  Jungle007
 *
 *  Created by ollie on 9/10/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "TestController.h"
#include "ofMain.h"
#include "ofEvents.h"

TestController::TestController(AppController *apc){
	_appController = (AppController *)apc;
	_facesPresent[0] = false;
	_facesPresent[1] = false;
	for(int i = 0; i < kTOTAL_TEST_EVENT_TYPES; i++){
		_timeLastEvent[i] = -1;
	}
	_lastTimeAnyEvent = -1;
}

bool TestController::fireEvent(rktEventType event){
	_timeLastEvent[event] = ofGetElapsedTimeMillis();
	if(event == face1ShowEvent){
		// trigger
//		ofNotify(apc->_camController->cameraEvent, apc->_camController, level);//
		return true;
	}
	if(event == face1HideEvent){
		return true;
	}
	if(event == face2ShowEvent){

		return true;
	}
	if(event == face2HideEvent){
		
		return true;
	}
	if(event == buttonEvent){
		float area = ofRandom(1.1); // TODO: What to use here?
		ofNotifyEvent(_appController->getArdController()->attackAction, area, _appController->getArdController());
		return true;
	}
	if(event == micEvent){
		
		return true;
	}

	ofAddListener(_appController->updateEvent, this, &TestController::update);

	return false;
}

void TestController::update(float & level){
	cout << "UPDATE IN TEST CONTROLLER" << endl;
	// Pick random event to trigger?
//	if(ofGetElapsedTimeMillis() > 5000){
//		fireEvent(buttonEvent);
//	}
}