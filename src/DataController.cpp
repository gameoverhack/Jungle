/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"

//#define USER_MATT
#define USER_OLLIE

DataController::DataController(string configFilePath) {

    LOG_NOTICE("Constructing DataController");

    registerStates();

	// used so we don't keep rebuilding on a parse error
	// (ie: rebuilt xml is faulty anyway, dont keep attempting)
	// member vairable instead of toggling parseRebuildXML property so we
	// accidentally saveout our change to the property.
	_hasAttemptedReparse = false;

	LOG_NOTICE("Initialising with " + configFilePath);
	_configFilePath = configFilePath;

	// fire and forget
	PropertyXMLParser propertyXMLParser(_configFilePath);

#if defined(USER_MATT)
    #ifdef TARGET_OSX
    _appModel->setProperty("scenesDataPath", (string)"/Users/gameover/Desktop/StrangerDanger/video");
    _appModel->setProperty("flashDataPath", (string)"/Users/gameover/Desktop/StrangerDanger/flash");
    _appModel->setProperty("graphicDataPath", (string)"graphics");
    #else TARGET_WIN32
    //_appModel->setProperty("scenesDataPath", (string)"E:/Users/gameover/Desktop/StrangerDanger/video");
    //_appModel->setProperty("flashDataPath", (string)"E:/Users/gameover/Desktop/StrangerDanger/flash");
    _appModel->setProperty("scenesDataPath", (string)"E:/gameoverload/VideoProjects/Jungle/newsleaze/video");
    _appModel->setProperty("flashDataPath", (string)"E:/gameoverload/VideoProjects/Jungle/newsleaze/flash");
    //_appModel->setProperty("scenesDataPath", (string)"D:/newmodel/video");
    //_appModel->setProperty("flashDataPath", (string)"D:/newmodel/flash");
    _appModel->setProperty("graphicDataPath", (string)"graphics");
    #endif
#else defined(USER_OLLIE)
    _appModel->setProperty("scenesDataPath", (string)"/Users/ollie/its_a_jungle_out_there/resources/video");
    _appModel->setProperty("flashDataPath", (string)"/Users/ollie/its_a_jungle_out_there/resources/flash");
    _appModel->setProperty("graphicDataPath", (string)"graphics");
#endif

#if !defined(USER_MATT) && !defined(USER_OLLIE)
    LOG_ERROR("I've made some defines for our user path - saves thrasing the config_props and I need to save them now...they're in DataController::DataController()");
    abort();
#endif

	if(boost::any_cast<bool>(_appModel->getProperty("xmlForceSceneBuildOnLoad"))){
		LOG_WARNING("Building XML due to property xmlForceSceneBuildOnLoad = true");
		buildXML();
	}

    _graphicLoader = new GraphicLoader(boost::any_cast<string>(_appModel->getProperty("graphicDataPath"))); // not doing this fancy for now ;-)

	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
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

				SceneXMLParser sceneParser = SceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
															boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
				setState(kDATACONTROLLER_FINISHED);
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
				}
			}
			catch (JungleException ex){
				// most likely get here if we can't fix a missing file issue
				LOG_ERROR("Caught JungleException: " + ex._message);
				LOG_ERROR("Don't know what to do, aborting.");
				abort();
			}
			break;

		case kDATACONTROLLER_SCENE_ANALYSING:

			if (_flashAnalyzer->checkState(kANAL_FINISHED)) {
				_flashAnalyzer->setState(kANAL_READY);
				LOG_WARNING("Rebuilding XML to handle mal-transforms");
				setState(kDATACONTROLLER_SCENE_PARSING);
				// reset to virgin state
				_hasAttemptedReparse = false;
				buildXML();
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
		//_appModel->setProperty("loadingMessage",  _sceneParser->printState(false));
	} else {
		_appModel->setProperty("loadingMessage",  _flashAnalyzer->getMessage());
	}
	//_appModel->setProperty("loadingProgress", _sceneParser->getLoadingProgress());
}

void DataController::runAnalyser(vector<string> files){
	LOG_NOTICE("Starting Analysis");
	_flashAnalyzer->setup(&files, 6667);
	setState(kDATACONTROLLER_SCENE_ANALYSING);
}

void DataController::buildXML(){
	// save existing file if present
	string sceneFileName = boost::any_cast<string>(_appModel->getProperty("scenesXMLFile"));
	string sceneFilePath = ofToDataPath(sceneFileName);
	char sceneBackupFileName[256];
	sprintf(sceneBackupFileName, "%s_%d_%d_%d_%dh%dm%ds.xml", (sceneFileName.substr(0, sceneFileName.find_last_of("."))).c_str(), ofGetYear(), ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
    string sceneBackupFilePath = ofToDataPath(sceneBackupFileName);

	ifstream in(sceneFilePath.c_str());
	ofstream out(sceneBackupFilePath.c_str());

	if(out == NULL) {
		LOG_ERROR("Could not backup XML file");
		abort();
	}
	if(in != NULL && out != NULL){
		out<< in.rdbuf();
		in.close();
		out.close();
	}
	// build
	try{
		SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),sceneFileName);
	}
	catch (AnalysisRequiredException ex) {

		LOG_WARNING("Analysis required due to builder throwing AnalysisRequiredException");

		string message = "";
		vector<string> brokenFiles = ex.getFiles();
        for (int i = 0; i < brokenFiles.size(); i++) {
            message = brokenFiles[i] + ", " + message;
        }

		message[message.length()-1] = ' ';

		LOG_ERROR("Require reanalysis/creation of transform files: " + message);

		if(boost::any_cast<bool>(_appModel->getProperty("xmlIgnoreTransformErrors"))){
			LOG_WARNING("xmlIgnoreTransformErrors true, continuing without rebuilding transforms");
		} else {
			if (!_hasAttemptedReparse) {
				runAnalyser(brokenFiles);
			} else {
				LOG_WARNING("hasAttemptedReparse already, continuing without rebuilding transforms");
			}

		}
	}


}

void DataController::rebuildXML(){
	if(_hasAttemptedReparse){
		// we've already tried to fix this once
		LOG_ERROR("Already attempted to rebuild XML once and failed, aborting.");
		abort();
	}

	buildXML();
	_hasAttemptedReparse = true; // don't loop re-trying to fix an error.
}
