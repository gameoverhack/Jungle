/*
 *  BaseController.cpp
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "BaseController.h"

BaseController::BaseController() {
	LOG_VERBOSE("Constructing a controller...");
	registerStates();
}

BaseController::~BaseController() {
	LOG_VERBOSE("Destructing a controller...");
	_states.clear();
	//_state = -1; // ??
}

void BaseController::registerStates() {
	LOG_VERBOSE("Registering defualt state: NONE == -1; over ride registerStates() to set your own states for this Controller");
	enum {
		NONE = -1
	};
	registerState(NONE, "NONE");
	setState(NONE);
	/*
	 
	// add registerStates() to all Controllers based on :
	
	// defining an enum:
	enum {
		SOMESTATE,
		ANOTHERSTATE,
		YETANOTHER
	 };
	 
	 // and registering them with:
	 
	 registerState(SOMESTATE, "SOMESTATE");
	 registerState(ANOTHERSTATE, "ANOTHERSTATE");
	 registerState(YETANOTHER, "YETANOTHER");
	 
	 */
	 
}

void BaseController::registerState(int intState, string strState) {
	LOG_VERBOSE("Adding STATE == " + strState + " == " + ofToString(intState));
	_states.insert(pair<int, string>(intState, strState));	// SHOULD WE JUST USE A SET AND MAKE SURE YOU REGISTER IN ORDER???? WOULD BE SIMPLER BUT NOT FAIL SAFE??
}

int BaseController::getState() {
	return _state;
}

void BaseController::setState(int state) {
	//assert(state < _states.size()); // more checks? different warn?
	LOG_VERBOSE("Setting STATE == " + _states.find(state)->second + " == " + ofToString(_states.find(state)->first));
	_state = state;
}

/*void BaseController::setState(string state) {
	
}*/

void BaseController::printState() {
	map<int, string>::iterator it = _states.find(_state);
	if (it == _states.end()) {
		LOG_ERROR("Cannot printState() -- no states have been registered or no state has been set"); // maybe don't need this now?
	} else {
		LOG_VERBOSE("STATE == " + it->second + " == " + ofToString(it->first));
	}
}

bool BaseController::checkState(int state) {
	//assert(state < _states.size()); // more checks? different warn?
	if (state == _state) {
		return true;
	} else return false;
}