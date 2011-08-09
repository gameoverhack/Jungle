/*
 *  DataController.h
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_DATACONTROLLER
#define _H_DATACONTROLLER

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/algorithm/string.hpp> // string splitting

#include <string>
using std::string;

#include "ofxXmlSettings.h"
#include "goDirList.h"

#include "BaseState.h"
#include "Logger.h"
#include "AppModel.h"
#include "AppDataTypes.h"
#include "Constants.h"
#include "SceneXMLBuilder.h"
#include "SceneXMLParser.h"
#include "PropertyXMLParser.h"
#include "PropertyXMLBuilder.h"
#include "Analyzer.h"

enum {
	kDATACONTROLLER_INIT,
	kDATACONTROLLER_SCENE_PARSING,
	kDATACONTROLLER_SCENE_ANALYSING,
	kDATACONTROLLER_FINISHED
};

class DataController : public BaseState {

public:
	
	DataController(string configFilePath);
	~DataController();
	
	void registerStates();
	
	void update();

	void saveProperties();

private:
	
	//string					_stateMessage;
	
	string					_configFilePath;
	
	SceneXMLParser			*_sceneParser;
	bool					_hasAttemptedReparse;

	void updateAppLoadingState();
	void rebuildXML();
	void restartParseXML();				

	
};


#endif