/*
 *  AppModel.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "AppModel.h"

// ctor & dtor
//--------------------------------------------------------------
AppModel::AppModel(){
	_currentScene = NULL;
	_padLength = 1;
	
	// setup video players
	_videoPlayers[0] = new goThreadedVideo();
	_videoPlayers[1] = new goThreadedVideo();
}

AppModel::~AppModel(){
	map<string, Scene *>::iterator iter;
	for(iter = _scenes.begin(); iter != _scenes.end(); iter++){
		delete (iter->second);
	}
	
	// clean up video players
	delete _videoPlayers[0];
	delete _videoPlayers[1];
}

// state registration
//--------------------------------------------------------------
void AppModel::registerStates() {
	LOG_VERBOSE("Registering States");
	
	registerState(kAPP_INIT, "kAPP_INIT");
	registerState(kAPP_LOADING, "kAPP_LOADING");
	registerState(kAPP_RUNNING, "kAPP_RUNNING");
	registerState(kAPP_EXITING, "kAPP_EXITING");
	
}

// scene getters and setters
//--------------------------------------------------------------
void AppModel::setScene(string sceneName, Scene * scene){
	_scenes.insert(pair<string, Scene *>(sceneName, scene));
}

Scene *AppModel::getScene(string sceneName){
	map<string, Scene *>::iterator iter;
	iter = _scenes.find(sceneName);
	if(iter == _scenes.end()){
		LOG_ERROR("Attempted to get invalid scene name " + sceneName);
		abort();
	}
	return iter->second;
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

// sequence getters and setters
//--------------------------------------------------------------
Sequence * AppModel::getCurrentSequence() {
	Sequence * seq;
	assert(_currentScene != NULL); // make sure _currentSequence is set (is this enough?)
	seq = _currentScene->getCurrentSequence();
	assert(seq != NULL);
	return seq;
}

// camera texture getters and setters
//--------------------------------------------------------------
void AppModel::setCameraTextures(ofTexture * victimCamTex, ofTexture * attackCamTex) {
	_victimCamTex = victimCamTex;
	_attackCamTex = attackCamTex;
}

ofTexture * AppModel::getVictimCamTexRef() {
	return _victimCamTex;
}

ofTexture * AppModel::getAttackCamTexRef() {
	return _attackCamTex;
}

// videoplayer getters and setters
//--------------------------------------------------------------
goThreadedVideo * AppModel::getCurrentVideoPlayer() {
	return _videoPlayers[0]; // always make 0 the current...
}

goThreadedVideo * AppModel::getNextVideoPlayer() {
	return _videoPlayers[1]; // ... and 1 the next or cached video player
}

void AppModel::toggleVideoPlayers() {
	LOG_VERBOSE("Swap Video Player pointers");
	_videoPlayers[1]->setPosition(0.0f);
	_videoPlayers[1]->update();
	swap(_videoPlayers[0], _videoPlayers[1]);
	_videoPlayers[1]->close();
	_videoPlayers[0]->psuedoUpdate(); // here? or in controller?
	
	delete _videoPlayers[1];
	_videoPlayers[1] = new goThreadedVideo();
}

void AppModel::setCurrentFrame(int frame) {
	//_frame = CLAMP(frame, 0, getCurrentFrameTotal()-1); //frame clamped to one less than total number;
	_frame = CLAMP(frame, 0, _currentScene->getCurrentSequence()->getTransformVector("atk1")->size()-1); // to be sure, to be sure!
}

int AppModel::getCurrentFrame() {
	return _frame;
}

int AppModel::getCurrentFrameTotal() { // for now no setter just do it on the actual movie....
	return _videoPlayers[0]->getTotalNumFrames();
}

void AppModel::setCurrentIsFrameNew(bool isFrameNew) {
	_isFrameNew = isFrameNew;
}

bool AppModel::getCurrentIsFrameNew() {
	return _isFrameNew;
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
		string propAsString = anyIT->first;
		if (is_int(anyIT->second) == true) {
			valAsString = ofToString(boost::any_cast<int>(anyIT->second));
		}
		if (is_float(anyIT->second) == true) {
			valAsString = ofToString(boost::any_cast<float>(anyIT->second));
		}
		if (is_string(anyIT->second) == true) {
			valAsString = boost::any_cast<string>(anyIT->second);
		}
		if(is_bool(anyIT->second) == true){
			valAsString = ((boost::any_cast<bool>)(anyIT->second) ? "true" : "false");
		}
		
		propsList += pad(propAsString) + " = " + pad(valAsString) + " type: " + anyIT->second.type().name() + "\n";
	}
	
	return propsList;
	
}

map<string, string> AppModel::getAllPropsNameTypeAsMap(){
	/*
	 returning as name,type instead of name,boost::any 
	 beacuse we'd have to expose the is_int/etc to check the any types.
	 */
	map<string, string> retmap;
	
	map<string, boost::any>::iterator iter;
	for (iter = _anyProps.begin(); iter != _anyProps.end(); iter++) {
		/* add the name and type to the map */		
		if (is_int(iter->second)){
			retmap.insert(pair<string,string>(iter->first, "int"));
		}
		if (is_float(iter->second)){
			retmap.insert(pair<string,string>(iter->first, "float"));
		}
		if (is_string(iter->second)){
			retmap.insert(pair<string,string>(iter->first, "string"));
		}
		if(is_bool(iter->second)){
			retmap.insert(pair<string,string>(iter->first, "bool"));
		}
	}
	return retmap;
}

inline string AppModel::pad(string & t_string) {
	
	// possibly a more elegant sprintf solution for this but can't work out how to
	// dynamically set sprintf(objWithWhiteSpace, "%" + ofToString(_padLength) + "s", objectName) ???
	
	string paddedString = t_string;
	int padLength = 0;
	
	// check length and adjust overall pad if the objectName is longer than the current padLength
	if (t_string.size() > _padLength) _padLength = t_string.size();
	
	padLength = _padLength - t_string.size();
	
	for (int i = 0; i < padLength; i++) paddedString += " ";
	
	return paddedString;

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

bool AppModel::is_bool(const boost::any & operand) {
	return operand.type() == typeid(bool);
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