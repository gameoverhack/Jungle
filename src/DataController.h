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

#include "BaseController.h"
#include "Logger.h"
#include "AppModel.h"
#include "AppDataTypes.h"
#include "Constants.h"
#include "SceneXMLBuilder.h"
#include "SceneXMLParser.h"
#include "PropertyXMLParser.h"
#include "PropertyXMLBuilder.h"

class DataController : public BaseController {

public:
	
	DataController(string configFilePath);
	~DataController();
	
	void update();
	
	DataControllerState getState();
	string getStateMessage();

	void saveProperties();

private:
	
	string					_stateMessage;
	DataControllerState		_state;
	
	string					_configFilePath;
	
	SceneXMLParser			*_sceneParser;
	bool					_hasAttemptedReparse;

	void updateAppLoadingState();
	void rebuildXML();
	void restartParseXML();				

	
};


#endif