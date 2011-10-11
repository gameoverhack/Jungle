/*
 *  KeyboardController.h
 *  Bunyip
 *
 *  Created by gameover on 23/08/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_KEYBOARDCONTROLLER
#define _H_KEYBOARDCONTROLLER

#include "ofMain.h"

#include "KeyboardModel.h"

class KeyboardController : public ofBaseApp { //, public BaseState {
	
public:
	
	KeyboardController();
	~KeyboardController();
	
private:
	
	void keyPressed  (ofKeyEventArgs &e);
	void keyReleased (ofKeyEventArgs &e);
	
};

#endif
