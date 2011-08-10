/*
 *  PropertyXMLParser.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_PROPERTYXMLPARSER
#define _H_PROPERTYXMLPARSER

#include "IXMLParser.h"
#include "AppModel.h"

class PropertyXMLParser : public IXMLParser {
public:
	PropertyXMLParser(string xmlFile);
	void parseXML();
};

#endif