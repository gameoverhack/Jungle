/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"

DataController::DataController(string configFilePath)
{
	LOG_NOTICE("Initialising with " + configFilePath);
	_configFilePath = configFilePath;

	// fire and forget
	PropertyXMLParser propertyXMLParser(_configFilePath);
//	PropertyXMLBuilder propertyXMLBuilder(_configFilePath);
	//SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
	//								boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));

	try {
		// also fire and forget
		SceneXMLParser sceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
								   boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));	
	}
	catch (JungleException je) {
		LOG_WARNING("Caught exception: " + je._message);
		LOG_NOTICE("Uncomment below to force rebuild of scene xml on parse failure");
		SceneXMLParser sceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));			
	}
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
	
}
