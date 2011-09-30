/*
 *  BaseState.cpp
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "BaseState.h"

BaseState::BaseState() {
	LOG_VERBOSE("Constructing a stateful class...");
	registerStates();
}

BaseState::~BaseState() {
	LOG_VERBOSE("Destructing a stateful class...");
	_states.clear();
	//_state = -1; // ??
}

void BaseState::registerStates() {
	LOG_VERBOSE("Registering defualt state: NONE == -1; over ride registerStates() to set your own states for this stateful class");
	registerState(NONE, "NONE");
	setState(NONE);
	/*
	 
	// add registerStates() to all Controllers based on :
	
	// defining an enum in the .h file for the class inheriting from BaseState:
	enum {
		SOMESTATE,
		ANOTHERSTATE,
		YETANOTHER
	 };
	 
	 // and registering them in registerStates() with:
	 
	 registerState(SOMESTATE, "SOMESTATE");
	 registerState(ANOTHERSTATE, "ANOTHERSTATE");
	 registerState(YETANOTHER, "YETANOTHER");
	 
	 NOTE: you have to specifically call registerStates() before setting, getting etc...as I couldn't get the base class to work how i wanted :-(
	 
	 */
	 
}

void BaseState::registerState(int intState, string strState) {
	LOG_VERBOSE("Adding STATE == " + strState + " == " + ofToString(intState));
	_states.insert(pair<int, string>(intState, strState));	// SHOULD WE JUST USE A SET AND MAKE SURE YOU REGISTER IN ORDER???? WOULD BE SIMPLER BUT NOT FAIL SAFE??
}

int BaseState::getState() {
	return _state;
}

void BaseState::setState(int state) {
	//assert(state < _states.size()); // more checks? different warn?
	LOG_VERBOSE("Setting STATE == " + _states.find(state)->second + " (" + ofToString(_states.find(state)->first) + ")");
	_state = state;
}

/*void BaseState::setState(string state) {
	
}*/

string BaseState::printState(bool log) {
	map<int, string>::iterator it = _states.find(_state);
	string msg;
	if (it == _states.end()) {
		msg = "Cannot printState() -- no states have been registered or no state has been set";
		if (log) LOG_ERROR(msg); // maybe don't need this now?
	} else {
		msg = "STATE == " + it->second + " (" + ofToString(it->first) + ")";
		if (log) LOG_VERBOSE(msg);
	}
	return msg;
}

bool BaseState::checkState(int state) {
	//assert(state < _states.size()); // more checks? different warn?
	if (state == _state) {
		return true;
	} else return false;
}