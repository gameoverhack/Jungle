/*
 *  IXMLBuilder.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_IXMLBUILDER
#define _H_IXMLBUILDER

#include "ofxXmlSettings.h"
#include "Logger.h"

//#include "tinyxmldump.h"

class IXMLBuilder {

public:
	IXMLBuilder(string xmlFile){
		_xmlFile = xmlFile;
	}
	
	virtual void buildXML() = 0; // must overide this
	
	bool save(){
		string filePath = ofToDataPath(_xmlFile, true);
		if(_xml.saveFile(filePath+"_temp.xml")){
			// remove the first file
			remove(filePath.c_str());
			// rename temp to final file
			rename((filePath+"_temp.xml").c_str(), filePath.c_str());
			return true;
		}
		else{
			LOG_ERROR("Could not save properties to xml. File error?");
			return false;
		}
	}
	
protected:
	string			_xmlFile;
	ofxXmlSettings	_xml;
				

};

#endif
