/*
 *  MicController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "MicController.h"

void MicController::registerStates() {
	LOG_VERBOSE("Registering States");

	registerState(kMICCONTROLLER_INIT, "kMICCONTROLLER_INIT");
	registerState(kMICCONTROLLER_BELOWTHRESHOLD, "kMICCONTROLLER_BELOWTHRESHOLD");
	registerState(kMICCONTROLLER_ABOVETHRESHOLD, "kMICCONTROLLER_ABOVETHRESHOLD");
	
	//setState(kMICCONTROLLER_INIT);
	setState(kMICCONTROLLER_BELOWTHRESHOLD);
}
