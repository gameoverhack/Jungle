/*
 *  IXMLParser.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_IXMLPARSER
#define _H_IXMLPARSER

#include "ofxXmlSettings.h"
#include "Logger.h"

class IXMLParser{

public:
	
	IXMLParser(string xmlFile){
		_xmlFile = xmlFile;
	}
	
	bool load(){
		// quick check
		if(!_xml.loadFile(ofToDataPath(_xmlFile))){
			LOG_ERROR("Could not load scene config: " + ofToDataPath(_xmlFile) + ". Is there XML in it?");
			abort(); //TODO This should throw exception instead
		}
	}

	virtual void parseXML() = 0;

protected:

	ofxXmlSettings	_xml;	
	string			_xmlFile;

};


#endif