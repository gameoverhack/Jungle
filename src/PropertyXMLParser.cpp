/*
 *  PropertyXMLParser.cpp
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "PropertyXMLParser.h"

PropertyXMLParser::PropertyXMLParser(string xmlFile) : IXMLParser(xmlFile){
	load();
	parseXML();
}

void PropertyXMLParser::parseXML(){

	_xml.pushTag("config"); // push into root.

	//  pull out the properties
	string defaultString = "this should never be seen"; // getValue requires type to know what to return
	string propName, propValue, propType;

	LOG_VERBOSE("Discovering properties");
	// Find all the property tags
	if(!_xml.tagExists("properties")){
		LOG_ERROR("No properties node in config");
		abort();
	}
	_xml.pushTag("properties"); // set properties as root node

	// loop over all property tags
	for(int i = 0; i < _xml.getNumTags("property"); i++){
		propName = _xml.getAttribute("property",
									"name", defaultString, i);
		propType = _xml.getAttribute("property",
									"type", defaultString, i);

		/*

		 Have to cast the returns from ofxXmlsettings::getValue for some reason
		 even though we pass in a default value to make it call the correct
		 function.

		 */
		if(propType == "float"){
			_appModel->setProperty(propName,
								   (float)(_xml.getValue("property",(float)0.0f, i)));
		}
		else if(propType == "int"){
			_appModel->setProperty(propName,
								   (int)(_xml.getValue("property",(int)1, i)));
		}
		else if(propType == "bool"){
			_appModel->setProperty(propName,
								   (bool)(_xml.getValue("property",defaultString, i) == "true" ? true : false));
		}
		else if(propType == "string"){
			_appModel->setProperty(propName,
								   (string)(_xml.getValue("property",defaultString, i)));
		}
		else{
			LOG_WARNING("Could not set property: " + propName + ", unkown type: " + propType);
		}
	}
	_xml.popTag();
}
