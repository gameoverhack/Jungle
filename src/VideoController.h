/*
 *  VideoController.h
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _VIDEOCONTROLLER_H
#define _VIDEOCONTROLLER_H

#include "BaseState.h"

enum {
	SOME,
	ANOTHER
};

class VideoController : public BaseState {

public:
	
	void	registerStates();
	
private:
	
};

#endif