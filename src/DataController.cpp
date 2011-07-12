/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"

DataController::DataController(string configFilePath){
	LOG_NOTICE("Initialising with " + configFilePath);

	propertyXMLParser(configFilePath);
	propertyXMLBuilder(configFilePath);

	// fire and forget
	SceneXMLBuilder sceneBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
								boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));

	// also fire and forget
	SceneXMLParser sceneParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
							   boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));	
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
}


bool DataController::propertyXMLParser(string propertyConfigFilepath){
	//  load up the property xml file 
	ofxXmlSettings xml;
	if(!xml.loadFile(propertyConfigFilepath)){
		LOG_ERROR("Could not load property config: " + propertyConfigFilepath);
		abort();		
	}
	
	xml.pushTag("config"); // push into root.
	//  pull out the properties 
	string defaultString = "this should never be seen"; // getValue requires type to know what to return
	string propName, propValue, propType;
	
	LOG_NOTICE("Discovering properties");
	// Find all the property tags
	if(!xml.tagExists("properties")){
		LOG_ERROR("No properties node in config");
		abort();
	}
	xml.pushTag("properties"); // set properties as root node
	
	// loop over all property tags
	for(int i = 0; i < xml.getNumTags("property"); i++){
		propName = xml.getAttribute("property",
									  "name", defaultString, i);
		propType = xml.getAttribute("property",
									  "type", defaultString, i);
		
		/*
		 
		 Have to cast the returns from ofxXmlsettings::getValue for some reason
		 even though we pass in a default value to make it call the correct
		 function.
		 
		 */
		if(propType == "float"){
			_appModel->setProperty(propName,
								   (float)(xml.getValue("property",(float)0.0f, i)));
		}
		else if(propType == "int"){
			_appModel->setProperty(propName,
								   (int)(xml.getValue("property",(int)1, i)));
		}
		else if(propType == "bool"){
			_appModel->setProperty(propName,
								   (bool)(xml.getValue("property",defaultString, i) == "true" ? true : false));
		}
		else if(propType == "string"){
			_appModel->setProperty(propName,
								   (string)(xml.getValue("property",defaultString, i)));
		}
		else{
			LOG_WARNING("Could not set property: " + propName + ", unkown type: " + propType);
		}
	} 
	xml.popTag();

	return true;
}

bool DataController::propertyXMLBuilder(string propertyConfigFilepath){
	int which; // used for ofxXmlSettings
	
	// get list of all properties 
	map<string, string> propsAsMap = _appModel->getAllPropsNameTypeAsMap();

	// set up xml
	ofxXmlSettings xml;
	xml.addTag("config");
	xml.pushTag("config");
	xml.addTag("properties");
	xml.pushTag("properties");

	// create nodes for each property 
	map<string, string>::iterator iter = propsAsMap.begin();
	while(iter != propsAsMap.end()){
		which = xml.addTag("property");
		xml.addAttribute("property", "name", iter->first, which); // name
		xml.addAttribute("property", "type", iter->second, which); // type

		// value
		if(iter->second == "float"){
			xml.setValue("property", boost::any_cast<float>(_appModel->getProperty(iter->first)), which);
		}
		else if(iter->second == "int"){
			xml.setValue("property", boost::any_cast<int>(_appModel->getProperty(iter->first)), which);
		}
		else if(iter->second == "bool"){
			xml.setValue("property", boost::any_cast<bool>(_appModel->getProperty(iter->first)) ? "true" : "false", which);
		}
		else if(iter->second == "string"){
			xml.setValue("property", boost::any_cast<string>(_appModel->getProperty(iter->first)), which);
		}
		else{
			LOG_WARNING("Could not save property: " + iter->first + ", unkown type: " + iter->second);
		}		
		iter++;
	}
	xml.popTag();
	xml.popTag();
	
	// save propertxml
	LOG_NOTICE("Saving properties to xml");
	if(xml.saveFile(propertyConfigFilepath+"_temp.xml")){
		// remove the first file
		remove(ofToDataPath(propertyConfigFilepath, true).c_str());
		// rename temp to final file
		rename(ofToDataPath(propertyConfigFilepath+"_temp.xml", true).c_str(), ofToDataPath(propertyConfigFilepath, true).c_str());
		return true;
	}
	else{
		LOG_ERROR("Could not save properties to xml. File error?");
		return false;
	}
}

bool DataController::sceneXMLParser(string sceneConfigFilePath){
}
