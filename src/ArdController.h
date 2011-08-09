/*
 *  ArdController.h
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_ARDCONTROLLER
#define _H_ARDCONTROLLER

#include "BaseState.h"

enum {
	kARDCONTROLLER_INIT,
	kARDCONTROLLER_BELOWTHRESHOLD,
	kARDCONTROLLER_ABOVETHRESHOLD
};

class ArdController : public BaseState {
	
public:
	
	void	registerStates();
	
private:
	
};

#endif