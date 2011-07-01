/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"
#include "tinyxml.h"

DataController::DataController(string configFilePath){
	LOG_NOTICE("Initialising with " + configFilePath);
	/* try to open the file */
	TiXmlDocument xmldoc(configFilePath);
	TiXmlElement *el;
	string propName, propValue, propType;
	
	if(!xmldoc.LoadFile()){ /* Capital letter method names wat. */
		LOG_ERROR("Could not load config: " + configFilePath);
		/*
			Not sure how we should be failing here, no global quit method in the app
			Don't want to assert because we want to log the failure.
		 */
		abort();
	}
	/* loaded the document */
	/* find the properties node */
	LOG_NOTICE("Discovering properties");
	el = xmldoc.RootElement()->FirstChildElement("properties");
	for(TiXmlElement *prop = el->FirstChildElement(); prop; prop = prop->NextSiblingElement()){
		LOG_NOTICE("Reading property: " + string(prop->Value())  +
				   "(" + string(prop->Attribute("type")) +
				   ")=> " + string(prop->FirstChild()->Value()));
		propName = string(prop->Value());
		propType = string(prop->Attribute("type"));
		propValue = string(prop->FirstChild()->Value());

		if(propType == "float"){
			_appModel->setProperty(propName, strtof(propValue.c_str(), NULL));
		}
		else if(propType == "int"){
			_appModel->setProperty(propName, strtol(propValue.c_str(), NULL, 10));
		}
		else if(propType == "bool"){
			_appModel->setProperty(propName, (propValue == "true" ? true : false));
		}
		else if(propType == "string"){
			_appModel->setProperty(propName, propValue);
		}
		else{
			LOG_WARNING("Could not set property: " + propName + ", unkown type: " + propType);
		}
	}
	LOG_NOTICE("Initialisation complete");
	
}

template <class vectorType>
void DataController::loadVector(string filePath, vector< vectorType > & vec) {
	ofLog(OF_LOG_VERBOSE, ("LoadVector: " + filePath));
	std::ifstream ifs(filePath.c_str());
    boost::archive::text_iarchive ia(ifs);
    ia >> vec;	
}
