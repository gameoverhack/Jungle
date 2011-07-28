/*
 *  VideoController.cpp
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "VideoController.h"

void VideoController::registerStates() {
	LOG_VERBOSE("Registering States");
	
	enum {
		SOME,
		ANOTHER
	};
	
	registerState(SOME, "SOME");
	registerState(ANOTHER, "ANOTHER");
	
	setState(ANOTHER);
	
	printState();
}