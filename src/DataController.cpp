/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"

DataController::DataController(string configFilePath)
{
	LOG_NOTICE("Initialising with " + configFilePath);
	_configFilePath = configFilePath;

	// fire and forget
	PropertyXMLParser propertyXMLParser(_configFilePath);

	_sceneParser = new SceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));	
	
	_state = kDATACONTROLLER_SCENE_PARSING;

	// used so we don't keep rebuilding on a parse error
	// (ie: rebuilt xml is faulty anyway, dont keep attempting)
	// member vairable instead of toggling parseRebuildXML property so we 
	// accidentally saveout our change to the property.
	_hasAttemptedReparse = false; 
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
	delete _sceneParser;
	
}

void DataController::update(){
	switch(_state){
		case kDATACONTROLLER_SCENE_PARSING:
			_stateMessage = _sceneParser->getStateMessage();
			try{
				// run scene parser update
				// this will look at the scene parsers internal state and perform 
				// whatever is necessary.
				_sceneParser->update();
			}
			catch(MetadataMismatchException ex){
				LOG_WARNING("Metadata mismatch exception: " + ex._message);
				
				// check if we want to handle it
				if(!boost::any_cast<bool>(_appModel->getProperty("parseRebuildXML")) &&
				   _hasAttemptedReparse){
					// dont rebuild, we had a parse error, cant continue.
					abort();
				}
				LOG_WARNING("Rebuilding XML to handle metadata mismatch exception");
				rebuildXML();
				restartParseXML();
			}
			catch (TransformMovieLengthMismatchException ex) {
				LOG_WARNING("Transform movie length mismatch exception: " + ex._message);
				if(!boost::any_cast<bool>(_appModel->getProperty("parseRequiresTransformReanalysis"))){
					LOG_WARNING("Continuing without rebuilding transforms");
				}
				
				LOG_ERROR("PUT START ANALYSE CODE HERE");
				abort();

			}
			catch (GenericXMLParseException ex) {
				LOG_WARNING("XML parse exception: " + ex._message);
				
				// check if we want to handle it
				if(!boost::any_cast<bool>(_appModel->getProperty("parseRebuildXML")) &&
				   _hasAttemptedReparse){
					// dont rebuild, we had a parse error, cant continue.
					abort();
				}

				LOG_WARNING("Rebuilding generic XML parse exception");
				rebuildXML();
				restartParseXML();
			}
			// if the parser is done, update our state
			if(_sceneParser->getState() == kSCENEXMLPARSER_FINISHED){
				_state = kDATACONTROLLER_FINISHED;
			}
			break;

		case kDATACONTROLLER_SCENE_ANALYSING:
			LOG_ERROR("MAKE DATA ANYALYSER HERE.");
			abort();

		default:
			LOG_ERROR("Unknown state " + _state);
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
	_appModel->setProperty("loadingMessage", _sceneParser->getStateMessage());
	_appModel->setProperty("loadingProgress", _sceneParser->getLoadingProgress());
}



DataControllerState DataController::getState(){
	return _state;
}
string DataController::getStateMessage(){
	return _stateMessage;
}


void DataController::rebuildXML(){
	// rebuild
	SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));	
}

void DataController::restartParseXML(){
	// delete old parser and make a new one (fresh state etc)
	delete _sceneParser;
	_sceneParser = new SceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
	_hasAttemptedReparse = true; // don't loop re-trying to fix an error.
}

