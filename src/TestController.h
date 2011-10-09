/*
 *  TestController.h
 *  Jungle007
 *
 *  Created by ollie on 9/10/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_TESTCONTROLLER
#define _H_TESTCONTROLLER

#include "Logger.h"
#include "ofMain.h"
#include "ofEvents.h"
#include "AppController.h"


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

#define kTOTAL_TEST_EVENT_TYPES (6)
typedef enum rktEventType {
	face1ShowEvent,
	face2ShowEvent,
	face1HideEvent,
	face2HideEvent,
	buttonEvent,
	micEvent
} rktEventType;

class TestController {
	
	public:
	
	TestController(AppController *apc);
	
	void update(float & level);
	
	void gotMessage(ofMessage message){
		cout << message.message;
	};

	bool fireEvent(rktEventType event);
	
	int getRandomEventType(){
		return (int)(ofRandom(kTOTAL_TEST_EVENT_TYPES));
	};
	
	
	void VictimEvent(float & level);
	
	AppController	*_appController;
		
	bool	_facesPresent[2];
	int		_timeLastEvent[kTOTAL_TEST_EVENT_TYPES];
	int		_lastTimeAnyEvent;
	
    ofEvent<float>  myEvent;
	
};

#endif