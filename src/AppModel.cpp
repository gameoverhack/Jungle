/*
 *  AppModel.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "AppModel.h"

AppModel::AppModel(){
	_currentScene = NULL;
}

AppModel::~AppModel(){
	map<string, Scene *>::iterator iter;
	for(iter = _scenes.begin(); iter != _scenes.end(); iter++){
		delete (iter->second);
	}
}

void AppModel::setScene(string sceneName, Scene * scene){
	_scenes.insert(pair<string, Scene *>(sceneName, scene));
}

bool AppModel::setCurrentScene(string sceneName){
	map<string, Scene *>::iterator iter;
	iter = _scenes.find(sceneName);
	if(iter != _scenes.end()){
		_currentScene = iter->second; // TODO: DO we have to dereference this? TEST IT.
		LOG_NOTICE("Set current scene to " + sceneName);
		return true;
	}
	else{
		LOG_ERROR("Unable to set current scene to '" + sceneName + "' because its not in the map");
	}
	return false;
}

bool AppModel::nextScene(){
	string nextname;
	// find current scene in map
	map<string, Scene *>::iterator iter;
	for(iter = _scenes.begin(); iter != _scenes.end(); iter++){
		if(iter->first == _currentScene->getName()){
			iter++; // found current, increment to next
			// is iter end? set name to first if so, else use iter name
			nextname = (iter == _scenes.end() ? _scenes.begin()->first : iter->first);
			setCurrentScene(nextname);
			LOG_VERBOSE("Nextscene: " + nextname);
			return true;
		}
	}
	return false; // should never get here, can probably just be void return
}

Scene * AppModel::getCurrentScene(){
	assert(_currentScene != NULL);
	return _currentScene;
}

Sequence * AppModel::getCurrentSequence() {
	Sequence * seq;
	assert(_currentScene != NULL); // make sure _currentSequence is set (is this enough?)
	seq = _currentScene->getCurrentSequence();
	assert(seq != NULL);
	return seq;
}

/********************************************************
 * Getters and setters for simple int, float, string	*
 * properties using boost::any and std::map. These		*
 * should NOT be used where efficiency really really	*
 * matters but still seem pretty quick!					*
 ********************************************************/

// set any property in a map to propVal
void AppModel::setProperty(string propName, boost::any propVal) {
	
	if (_anyProps.count(propName) != 0 && !_anyProps[propName].empty()) {
		assert (_anyProps[propName].type() == propVal.type());	// don't let properties change types once init'd
	}
	
	_anyProps[propName] = propVal;
	
}

// get any int property in a map (no cast necessary)
void AppModel::getProperty(string propName, int & propVal) {
	
	assert(_anyProps.count(propName) != 0);	// if it ain't there abort
	assert(is_int(_anyProps[propName]));		// if it propVal ref is not same type abort
	propVal = boost::any_cast<int>(_anyProps[propName]);
	
}

// get any float property in a map (no cast necessary)
void AppModel::getProperty(string propName, float & propVal) {
	
	assert(_anyProps.count(propName) != 0);	// if it ain't there abort
	assert(is_float(_anyProps[propName]));	// if it propVal ref is not same type abort
	propVal = boost::any_cast<float>(_anyProps[propName]);
	
}

// get any string property in a map (no cast necessary)
void AppModel::getProperty(string propName, string & propVal) {
	
	assert(_anyProps.count(propName) != 0);	// if it ain't there abort
	assert(is_string(_anyProps[propName]));	// if it propVal ref is not same type abort
	propVal = boost::any_cast<string>(_anyProps[propName]);
	
}

// get any ANY property in a map (cast necessary -> use boost::any_cast<TYPE>(property))
boost::any AppModel::getProperty(string propName) {
	
	assert(_anyProps.count(propName) != 0); // if it ain't there abort
	return _anyProps[propName];
	
}

// return a list of ALL properties -> useful for debug
string AppModel::getAllPropsAsList() {
	
	string propsList;
	
	map<string, boost::any>::iterator anyIT;
	for (anyIT = _anyProps.begin(); anyIT != _anyProps.end(); anyIT++) {
		
		string valAsString;

		if (is_int(anyIT->second) == true) {
			valAsString = ofToString(boost::any_cast<int>(anyIT->second));
		}
		if (is_float(anyIT->second) == true) {
			valAsString = ofToString(boost::any_cast<float>(anyIT->second));
		}
		if (is_string(anyIT->second) == true) {
			valAsString = boost::any_cast<string>(anyIT->second);
		}
		
		propsList += anyIT->first + " = " + valAsString + " type: " + anyIT->second.type().name() + "\n";
	}
	
	return propsList;
	
}

// check type int
bool AppModel::is_int(const boost::any & operand) {
    return operand.type() == typeid(int);
}

// check type float
bool AppModel::is_float(const boost::any & operand) {
    return operand.type() == typeid(float);
}

// check type string
bool AppModel::is_string(const boost::any & operand) {
    return operand.type() == typeid(string);
}

// check type char *
bool AppModel::is_char_ptr(const boost::any & operand) {
    try
    {
		boost::any_cast<const char *>(operand);
        return true;
    }
    catch(const boost::bad_any_cast &)
    {
        return false;
    }
}