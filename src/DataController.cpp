/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"

DataController::DataController(string configFilePath) {
	
	// used so we don't keep rebuilding on a parse error
	// (ie: rebuilt xml is faulty anyway, dont keep attempting)
	// member vairable instead of toggling parseRebuildXML property so we
	// accidentally saveout our change to the property.
	_hasAttemptedReparse = false;
	
	LOG_NOTICE("Initialising with " + configFilePath);
	_configFilePath = configFilePath;
	
	// fire and forget
	PropertyXMLParser propertyXMLParser(_configFilePath);
	
	if(boost::any_cast<bool>(_appModel->getProperty("xmlForceSceneBuildOnLoad"))){
		LOG_WARNING("Building XML due to property xmlForceSceneBuildOnLoad = true");
		rebuildXML();
	}
	
	_sceneParser = new SceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
	_sceneParser->registerStates();
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
	delete _sceneParser;
	
}

void DataController::registerStates() {
	LOG_VERBOSE("Registering States");
	
	registerState(kDATACONTROLLER_INIT, "kDATACONTROLLER_INIT");
	registerState(kDATACONTROLLER_SCENE_PARSING, "kDATACONTROLLER_SCENE_PARSING");
	registerState(kDATACONTROLLER_SCENE_ANALYSING, "kDATACONTROLLER_SCENE_ANALYSING");
	registerState(kDATACONTROLLER_FINISHED, "kDATACONTROLLER_FINISHED");
	
	setState(kDATACONTROLLER_SCENE_PARSING);
}

void DataController::update(){
	
	switch(getState()){
		case kDATACONTROLLER_SCENE_PARSING:
			try{
				// run scene parser update
				// this will look at the scene parsers internal state and perform
				// whatever is necessary.
				_sceneParser->update();
			}
			catch(MetadataMismatchException ex){
				LOG_WARNING("Metadata mismatch exception: " + ex._message);
				
				// check if we want to handle it
				if(boost::any_cast<bool>(_appModel->getProperty("xmlIgnoreErrors"))){
					LOG_WARNING("xmlIgnoreErrors true, ignoring exception");
					// ignoring issue
				}
				else {
					// try to rebuild
					LOG_WARNING("Rebuilding XML to handle metadata mismatch exception");
					rebuildXML();
					restartParseXML();
				}
				
			}
			catch (TransformMovieLengthMismatchException ex) {
				string message = "";
				for(vector<string>::iterator iter = ex._names.begin(); iter != ex._names.end(); iter++){
					message = *iter + ", " + message;
				}
				message[message.length()-1] = ' ';
				LOG_ERROR("Require reanalysis/creation of transform files: " + message);
				
				if(boost::any_cast<bool>(_appModel->getProperty("xmlIgnoreTransformErrors"))){
					LOG_WARNING("xmlIgnoreTransformErrors true, continuing without rebuilding transforms");
				} else {
					if (!_hasAttemptedReparse) {
						LOG_NOTICE("Starting Analysis");
						_flashAnalyzer->setup(&ex._names, 6667);
						setState(kDATACONTROLLER_SCENE_ANALYSING);
					} else {
						LOG_WARNING("hasAttemptedReparse already, continuing without rebuilding transforms");
					}

				}
				
			}
			catch (GenericXMLParseException ex) {
				LOG_WARNING("XML parse exception: " + ex._message);
				
				// check if we want to handle it
				if(boost::any_cast<bool>(_appModel->getProperty("xmlIgnoreErrors"))){
					LOG_WARNING("xmlIgnoreErrors true, ignoring exception");
					// ignore issue
				} else {
					LOG_WARNING("Rebuilding XML to handle parse exception");
					rebuildXML();
					restartParseXML();
				}
			}
			catch (JungleException ex){
				// most likely get here if we can't fix a missing file issue
				LOG_ERROR("Caught JungleException: " + ex._message);
				LOG_ERROR("Don't know what to do, aborting.");
				abort();
			}
			// if the parser is done, update our state
			if(_sceneParser->checkState(kSCENEXMLPARSER_FINISHED)){
				setState(kDATACONTROLLER_FINISHED);
			}
			break;
			
		case kDATACONTROLLER_SCENE_ANALYSING:
			if (_flashAnalyzer->checkState(kANAL_FINISHED)) {
				_flashAnalyzer->setState(kANAL_READY);
				LOG_WARNING("Rebuilding XML to handle mal-transforms");
				setState(kDATACONTROLLER_SCENE_PARSING);
				rebuildXML();
				restartParseXML();
			} else _flashAnalyzer->update();
			
			break;
		case kDATACONTROLLER_FINISHED:
			LOG_VERBOSE("Data controller is finished.");
			break;
		default:
			LOG_ERROR("Unknown state " + printState());
	}
	updateAppLoadingState();
}

void DataController::saveProperties(){
	LOG_NOTICE("Saving properties to XML");
	PropertyXMLBuilder propertyXMLBuilder(_configFilePath);
}

// Convenience function
void DataController::updateAppLoadingState(){
	// loading messave is just the scene parser state state.
	if(!kDATACONTROLLER_SCENE_ANALYSING) {
		_appModel->setProperty("loadingMessage",  _sceneParser->printState());
	} else {
		_appModel->setProperty("loadingMessage",  _flashAnalyzer->getMessage());
	}
	_appModel->setProperty("loadingProgress", _sceneParser->getLoadingProgress());
}

void DataController::rebuildXML(){
	if(_hasAttemptedReparse){
		// we've already tried to fix this once
		LOG_ERROR("Already attempted to rebuild XML once and failed, aborting.");
		abort();
	}	
	// rebuild
	SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
}

void DataController::restartParseXML(){
	// delete old parser and make a new one (fresh state etc)
	delete _sceneParser;
	_sceneParser = new SceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
	_sceneParser->registerStates();
	_hasAttemptedReparse = true; // don't loop re-trying to fix an error.
}

