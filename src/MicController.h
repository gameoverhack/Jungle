/*
 *  MicController.h
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_MICCONTROLLER
#define _H_MICCONTROLLER

#include "BaseState.h"

enum {
	kMICCONTROLLER_INIT,
	kMICCONTROLLER_BELOWTHRESHOLD,
	kMICCONTROLLER_ABOVETHRESHOLD
};

class MicController : public BaseState {

public:

	void	registerStates();
	
private:
	
};

#endif