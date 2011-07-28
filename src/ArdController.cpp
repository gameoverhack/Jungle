/*
 *  ArdController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "ArdController.h"

void ArdController::registerStates() {
	LOG_VERBOSE("Registering States");
	
	registerState(kARDCONTROLLER_INIT, "kARDCONTROLLER_INIT");
	registerState(kARDCONTROLLER_BELOWTHRESHOLD, "kARDCONTROLLER_BELOWTHRESHOLD");
	registerState(kARDCONTROLLER_ABOVETHRESHOLD, "kARDCONTROLLER_ABOVETHRESHOLD");
	
	//setState(kARDCONTROLLER_INIT);
	setState(kARDCONTROLLER_BELOWTHRESHOLD);
}