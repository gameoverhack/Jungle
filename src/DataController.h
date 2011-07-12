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

#include "Logger.h"
#include "AppModel.h"
#include "AppDataTypes.h"
#include "Constants.h"
#include "SceneXMLBuilder.h"
#include "SceneXMLParser.h"

class DataController {

public:
	DataController(string configFilePath);
	~DataController();
	
	bool propertyXMLBuilder(string propertyConfigFilepath);
	bool propertyXMLParser(string propertyConfigFilepath);

	bool sceneXMLParser(string configFilePath);
	
private :
	ofxXmlSettings * _xml;
	
};


#endif