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
AppModel::AppModel() {

    LOG_NOTICE("Constructing AppModel");

    registerStates();

	_currentScene = NULL;
	_padLength = 1;

    _isFacePresent[0] = _isFacePresent[1] = false;



	// setup video players
	for (int i = 0; i < 2; i++) {
	    _fakePlayers[i] = new goVideoPlayer();
	    _fakePlayers[i]->setPixelType(GO_TV_RGB);
		_videoPlayers[i] = new goThreadedVideo();
        _videoPlayers[i]->setPixelType(GO_TV_RGBA);
	}

}

AppModel::~AppModel() {

    LOG_NOTICE("Destroying AppModel");

	map<string, Scene *>::iterator iter;
	for(iter = _scenes.begin(); iter != _scenes.end(); iter++){
		delete (iter->second);
	}

	// clean up video players
	for (int i = 0; i < 2; i++) {
	    delete _fakePlayers[i];
	    delete _videoPlayers[i];
	}
	delete [] _cameraPRS;
	delete [] _ardCyclicBuffer;
	delete [] _fftCyclicBuffer;
	delete [] _fftCyclicSum;
	delete [] _fftInput;
	delete [] _fftNoiseFloor;
	delete [] _fftPostFilter;

    // there are more to do

    LOG_WARNING("Have you cleaned up the model????");

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

void AppModel::clearScenesAndSequences(){
	_currentScene = NULL;
	_scenes.clear();
}

/********************************************************
 *      Getters and setters for Scenes               	*
 ********************************************************/
//--------------------------------------------------------------
void AppModel::setScene(string sceneName, Scene * scene){
	_scenes.insert(pair<string, Scene *>(sceneName, scene));
}

Scene *AppModel::getScene(string sceneName){
	map<string, Scene *>::iterator iter;
	iter = _scenes.find(sceneName);
	if(iter == _scenes.end()){
		LOG_ERROR("Attempted to get invalid scene name " + sceneName);
		return NULL;
	}
	return iter->second;
}

bool AppModel::setCurrentScene(string sceneName){
	map<string, Scene *>::iterator sceneIter;
	sceneIter = _scenes.find(sceneName);
	if(sceneIter != _scenes.end()) {

		_currentScene = sceneIter->second; // TODO: DO we have to dereference this? TEST IT.

		LOG_NOTICE("Set current scene to " + sceneName);

        _currentSceneFrame = _lastSequenceFrame = 0;
        //_currentScene->setSequenceThresholds();

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
	if(_currentScene == NULL){
		setCurrentScene(_scenes.begin()->first);
		return true;
	}
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

/********************************************************
 *      Getters and setters for Sequence               	*
 ********************************************************/
//--------------------------------------------------------------
Sequence * AppModel::getCurrentSequence() {
	Sequence * seq;
	assert(_currentScene != NULL); // make sure _currentSequence is set (is this enough?)
	seq = _currentScene->getCurrentSequence();
	assert(seq != NULL);
	return seq;
}

/********************************************************
 *      Getters and setters for Cameras                	*
 ********************************************************/
//--------------------------------------------------------------
void AppModel::setCameraTextures(ofTexture * victimCamTex, ofTexture * attackCamTex) {
	_victimCamTex = victimCamTex;
	_attackCamTex = attackCamTex;
}

//--------------------------------------------------------------
ofTexture * AppModel::getVictimCamTexRef() {
	return _victimCamTex;
}

//--------------------------------------------------------------
ofTexture * AppModel::getAttackCamTexRef() {
	return _attackCamTex;
}

//--------------------------------------------------------------
void AppModel::setCameraAttributes(int which, PosRotScale * prs) {
    delete _cameraPRS[which];
    _cameraPRS[which] = prs;
}

//--------------------------------------------------------------
void AppModel::setFakeAttributes(int which, PosRotScale * prs) {
    delete _fakePRS[which];
    _fakePRS[which] = prs;
}

//--------------------------------------------------------------
PosRotScale * AppModel::getCameraAttributes(int which) {
    return _cameraPRS[which];
}

//--------------------------------------------------------------
PosRotScale * AppModel::getFakeAttributes(int which) {
    return _fakePRS[which];
}

//--------------------------------------------------------------
ofTexture * AppModel::getFakeVictimCamTexRef() {
	return &(_fakePlayers[0]->getTextureReference());
}

//--------------------------------------------------------------
ofTexture * AppModel::getFakeAttackCamTexRef() {
	return &(_fakePlayers[1]->getTextureReference());
}

//--------------------------------------------------------------
goVideoPlayer * AppModel::getFakeVictimPlayer() {
	return _fakePlayers[0];
}

//--------------------------------------------------------------
goVideoPlayer * AppModel::getFakeAttackPlayer() {
	return _fakePlayers[1];
}

/********************************************************
 *      Getters and setters for GraphicAssets       	*
 ********************************************************/
 //--------------------------------------------------------------
bool AppModel::loadGraphicAsset(string path, int type) {
    assert(type < kGFX_TOTAL); // will this do?
    return _gfxAssets[type].loadImage(ofToDataPath(path));
}

//--------------------------------------------------------------
ofTexture * AppModel::getGraphicTex(int type) {
    return &(_gfxAssets[type].getTextureReference());
}

/********************************************************
 *      Getters and setters for MicController       	*
 ********************************************************/

//--------------------------------------------------------------
void AppModel::setFFTArea(float area) {
    _fftArea = area;
}

//--------------------------------------------------------------
float AppModel::getFFTArea() {
    return _fftArea;
}

//--------------------------------------------------------------
void AppModel::setFFTBinSize(int size) {
    _fftBinSize = size;
}

//--------------------------------------------------------------
int AppModel::getFFTBinSize(){
    return _fftBinSize;
}

//--------------------------------------------------------------
void AppModel::setFFTCyclicBufferSize(int size) {
    _fftCyclicBufferSize = size;
}

//--------------------------------------------------------------
int AppModel::getFFTCyclicBufferSize(){
    return _fftCyclicBufferSize;
}

//--------------------------------------------------------------
void AppModel::setAudioBufferSize(int size){
    _audioBufferSize = size;
}

//--------------------------------------------------------------
int AppModel::getAudioBufferSize(){
    return _audioBufferSize;
}

//--------------------------------------------------------------
void AppModel::allocateFFTCyclicBuffer(int fftCyclicBufferSize, int fftBinSize) {
    _fftCyclicBuffer = new fftBands[fftCyclicBufferSize];
    for (int i = 0; i < fftCyclicBufferSize; i++) {
        _fftCyclicBuffer[i].fftBand = new float[fftBinSize];
        memset(_fftCyclicBuffer[i].fftBand, 0, sizeof(float) * fftBinSize);
    }
}

//--------------------------------------------------------------
void AppModel::allocateFFTNoiseFloor(int fftBinSize) {
    _fftNoiseFloor = new float[fftBinSize];
    memset(_fftNoiseFloor, 0, sizeof(float) * fftBinSize);
}

//--------------------------------------------------------------
void AppModel::allocateFFTCyclicSum(int fftBinSize) {
    _fftCyclicSum = new float[fftBinSize];
    memset(_fftCyclicSum, 0, sizeof(float) * fftBinSize);
}

//--------------------------------------------------------------
void AppModel::allocateFFTPostFilter(int fftBinSize) {
    _fftPostFilter = new float[fftBinSize];
    memset(_fftPostFilter, 0, sizeof(float) * fftBinSize);
}

//--------------------------------------------------------------
void AppModel::allocateFFTInput(int fftBinSize) {
    _fftInput = new float[fftBinSize];
    memset(_fftInput, 0, sizeof(float) * fftBinSize);
}

//--------------------------------------------------------------
void AppModel::allocateAudioInput(int bufferSize) {
    _audioInput = new float[bufferSize];
    memset(_audioInput, 0, sizeof(float) * bufferSize);
}

//--------------------------------------------------------------
fftBands * AppModel::getFFTCyclicBuffer() {
    return _fftCyclicBuffer;
}

//--------------------------------------------------------------
float * AppModel::getFFTNoiseFloor() {
    return _fftNoiseFloor;
}

//--------------------------------------------------------------
float * AppModel::getFFTCyclicSum() {
    return _fftCyclicSum;
}

//--------------------------------------------------------------
float * AppModel::getFFTPostFilter() {
    return _fftPostFilter;
}

//--------------------------------------------------------------
float * AppModel::getFFTInput() {
    return _fftInput;
}

//--------------------------------------------------------------
float * AppModel::getAudioInput() {
    return _audioInput;
}

/********************************************************
 *      Getters and setters for ArdController        	*
 ********************************************************/
//--------------------------------------------------------------
void AppModel::allocatePinInput(int numPins) {
    //delete _ardRawPins;
    _ardRawPins = new int[numPins];
}

//--------------------------------------------------------------
int * AppModel::getPinInput() {
    return _ardRawPins;
}

void AppModel::setARDArea(float area) {
    _ardArea = area;
}

//--------------------------------------------------------------
float AppModel::getARDArea() {
    return _ardArea;
}

//--------------------------------------------------------------
void AppModel::setARDCyclicBufferSize(int size) {
    _ardCyclicBufferSize = size;
}

//--------------------------------------------------------------
int AppModel::getARDCyclicBufferSize(){
    return _ardCyclicBufferSize;
}

//--------------------------------------------------------------
void AppModel::allocateARDCyclicBuffer(int ardCyclicBufferSize) {
    _ardCyclicBuffer = new float[ardCyclicBufferSize];
    memset(_ardCyclicBuffer, 0, sizeof(float) * ardCyclicBufferSize);
}

//--------------------------------------------------------------
void AppModel::allocateARDNoiseFloor() {
    _ardNoiseFloor = new float;
}

//--------------------------------------------------------------
void AppModel::allocateARDCyclicSum() {
    _ardCyclicSum = new float;
}

//--------------------------------------------------------------

float * AppModel::getARDCyclicBuffer() {
    return _ardCyclicBuffer;
}

//--------------------------------------------------------------
float * AppModel::getARDNoiseFloor() {
    return _ardNoiseFloor;
}

//--------------------------------------------------------------
float * AppModel::getARDCyclicSum() {
    return _ardCyclicSum;
}

//--------------------------------------------------------------
void AppModel::setARDPostFilter(float val) {
    _ardPostFilter = val;
}

//--------------------------------------------------------------
float AppModel::getARDPostFilter() {
    return _ardPostFilter;
}

/********************************************************
 *      Getters and setters for VideoController       	*
 ********************************************************/
//--------------------------------------------------------------
goThreadedVideo * AppModel::getCurrentVideoPlayer() {
	return _videoPlayers[0]; // always make 0 the current...
}

//--------------------------------------------------------------
goThreadedVideo * AppModel::getNextVideoPlayer() {
	return _videoPlayers[1]; // ... and 1 the next or cached video player
}

//--------------------------------------------------------------
void AppModel::toggleVideoPlayers(int forceFrame) {
	LOG_VERBOSE("Swap Video Player pointers: " + ofToString(forceFrame));
	//if (forceFrame != 0) {
    _videoPlayers[1]->setFrame(forceFrame);
	//_videoPlayers[1]->setPosition(0.0f);
	_videoPlayers[1]->update();
	swap(_videoPlayers[0], _videoPlayers[1]);
	_videoPlayers[1]->close();
	_videoPlayers[0]->psuedoUpdate(); // here? or in controller?
    _videoPlayers[0]->setFrame(forceFrame);
	delete _videoPlayers[1];
	_videoPlayers[1] = new goThreadedVideo();
	_videoPlayers[1]->setPixelType(GO_TV_RGBA);
}

//--------------------------------------------------------------
void AppModel::setCurrentSequenceFrame(int frame) {
	//_frame = CLAMP(frame, 0, getCurrentFrameTotal()-1); //frame clamped to one less than total number;
	_currentSequenceFrame = CLAMP(frame, 0, _currentScene->getCurrentSequence()->getTransformVector("atk1")->size()-1); // to be sure, to be sure!
	if (_currentScene->getCurrentSequence()->getType() == "a" && _currentScene->getCurrentSequence()->getNumber() > 0) {
	    if (_currentSequenceFrame > 0) _currentSceneFrame = _currentSceneFrame + _currentSequenceFrame - _lastSequenceFrame;
	    _lastSequenceFrame = _currentSequenceFrame;
	}
	setCurrentInteractivity(_currentSequenceFrame); // see notes below in Interactivity getter/setters section as to why this is here
}

//--------------------------------------------------------------
int AppModel::getCurrentSequenceFrame() {
	return _currentSequenceFrame;
}

//--------------------------------------------------------------
int AppModel::getCurrentSceneFrame() {
	return _currentSceneFrame; //not including loops and bs
}

//--------------------------------------------------------------
int AppModel::getCurrentSequenceNumFrames() {
	return _currentScene->getCurrentSequence()->getNumFrames(); //_videoPlayers[0]->getTotalNumFrames();
}

//--------------------------------------------------------------
int AppModel::getCurrentSceneNumFrames() {
	return _currentScene->getTotalFrames(); //not including loops and bs
}

//--------------------------------------------------------------
void AppModel::setCurrentIsFrameNew(bool isFrameNew) {
	_isFrameNew = isFrameNew;
}

//--------------------------------------------------------------
bool AppModel::getCurrentIsFrameNew() {
	return _isFrameNew;
}

/********************************************************
 *      Getters and setters for Interactivity       	*
 ********************************************************/
//--------------------------------------------------------------
bool AppModel::checkCurrentInteractivity(interaction_t interactionType) {
	return (getCurrentInteractivity() == interactionType);
}

//--------------------------------------------------------------
void AppModel::setCurrentInteractivity(int frame) {
	Sequence * seq						= getCurrentSequence();
	interaction_t * interactionTable	= seq->getInteractionTable();
	//int currentFrame					= getCurrentFrame(); // could use this if we called from outside model, but for now setCurrentFrame calls this method
	// should i do any checks here????
	_currentInteractivity = interactionTable[frame];
}

//--------------------------------------------------------------
int AppModel::getCurrentInteractivity() {

	/*Sequence * seq						= getCurrentSequence();
	interaction_t * interactionTable	= seq->getInteractionTable();
	int currentFrame					= getCurrentFrame();
	// should i do any checks here????
	return interactionTable[currentFrame];*/

	// have to setCurrentInteractivity per frame at some point
	// becuase we need to know when drawing views (meters, etc)
	// better to set this once per frame draw than to check it
	// every frame processed...so call setCurrentInteractivity
	// immediately after set frame...gonna do it internally for
	// now, but maybe better explicit in the videocontroller???

	return _currentInteractivity;
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

// has any ANY property in a map?
bool AppModel::hasProperty(string propName) {

	return (_anyProps.count(propName) == 0 ? false : true);

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

void AppModel::printAllScenes(){
	map<string, Scene *>::iterator iter = _scenes.begin();
	while(iter != _scenes.end()){
		iter->second->print();
		iter++;
	}
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
