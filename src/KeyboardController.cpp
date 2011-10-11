/*
 *  KeyboardController.cpp
 *  Bunyip
 *
 *  Created by gameover on 23/08/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "KeyboardController.h"

//--------------------------------------------------------------
KeyboardController::KeyboardController() {
	
	LOG_NOTICE("Constructing KeyboardController");
	
	ofAddListener(ofEvents.keyPressed, this, &KeyboardController::keyPressed);
    ofAddListener(ofEvents.keyReleased, this, &KeyboardController::keyReleased);
	
}

//--------------------------------------------------------------
KeyboardController::~KeyboardController() {
	
	LOG_NOTICE("Destructing KeyboardController");
	
	ofRemoveListener(ofEvents.keyPressed, this, &KeyboardController::keyPressed);
    ofRemoveListener(ofEvents.keyReleased, this, &KeyboardController::keyReleased);
	
}

//--------------------------------------------------------------
void KeyboardController::keyPressed(ofKeyEventArgs &e) {
	_keyModel->executeFunction(e.key, kKEY_DOWN);
}

//--------------------------------------------------------------
void KeyboardController::keyReleased(ofKeyEventArgs &e) {
	_keyModel->executeFunction(e.key, kKEY_UP);
}

