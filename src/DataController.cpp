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
//	PropertyXMLBuilder propertyXMLBuilder(_configFilePath);
	
//	SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
//									boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));			
	_sceneParser = new SceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));	
	
	_state = kDATACONTROLLER_SCENE_PARSING;
	
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
				LOG_NOTICE("Caught exception: " + ex._message);
				SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
												boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));			
			}
			catch (vector<string> exVec) { // TODO: This should probably be its own exception type 
				LOG_ERROR("TransformMovieLength error, rebuild required");
				abort();
			}
			catch (JungleException je) {
				LOG_WARNING("Caught exception: " + je._message);
				LOG_NOTICE("Uncomment below to force rebuild of scene xml on parse failure");
				abort();
				//		SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
				//									 boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
				//		SceneXMLParser sceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
				//									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));			
			}
			
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



