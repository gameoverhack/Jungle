/*
 *  PropertyXMLBuilder.cpp
 *  Jungle
 *
 *  Created by ollie on 13/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "PropertyXMLBuilder.h"

PropertyXMLBuilder::PropertyXMLBuilder(string xmlFile) : IXMLBuilder(xmlFile){
	buildXML();
	save();
}

void PropertyXMLBuilder::buildXML(){
	int which; // used for ofxXmlSettings

	// get list of all properties
	map<string, string> propsAsMap = _appModel->getAllPropsNameTypeAsMap();

	// set xml and root
	_xml.addTag("config");
	_xml.pushTag("config");
	_xml.addTag("properties");
	_xml.pushTag("properties");

	// create nodes for each property
	map<string, string>::iterator iter = propsAsMap.begin();
	while(iter != propsAsMap.end()){
		which = _xml.addTag("property");
		_xml.addAttribute("property", "name", iter->first, which); // name
		_xml.addAttribute("property", "type", iter->second, which); // type

		// value
		if(iter->second == "float"){
			_xml.setValue("property", boost::any_cast<float>(_appModel->getProperty(iter->first)), which);
		}
		else if(iter->second == "int"){
			_xml.setValue("property", boost::any_cast<int>(_appModel->getProperty(iter->first)), which);
		}
		else if(iter->second == "bool"){
			_xml.setValue("property", boost::any_cast<bool>(_appModel->getProperty(iter->first)) ? "true" : "false", which);
		}
		else if(iter->second == "string"){
			_xml.setValue("property", boost::any_cast<string>(_appModel->getProperty(iter->first)), which);
		}
		else{
			LOG_WARNING("Could not save property: " + iter->first + ", unkown type: " + iter->second);
		}
		iter++;
	}
	_xml.popTag();
	_xml.popTag();
}
