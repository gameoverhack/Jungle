/*
 *  PropertyXMLBuilder.h
 *  Jungle
 *
 *  Created by ollie on 13/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_PROPERTYXMLBUILDER
#define _H_PROPERTYXMLBUILDER

#include "IXMLBuilder.h"
#include "AppModel.h"

class PropertyXMLBuilder : public IXMLBuilder {
public:
	PropertyXMLBuilder(string xmlFile);
	void buildXML();
};

#endif