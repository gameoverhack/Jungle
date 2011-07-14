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

	bool attemptParse = true;
//	SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
//									boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));			

	
	while(attemptParse){
		try {
			// also fire and forget
			SceneXMLParser sceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
										  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));	
		}
		catch(MetadataMismatchException ex){
			LOG_NOTICE("Caught exception: " + ex._message);
			SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
											boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));			
		}
		catch (JungleException je) {
			LOG_WARNING("Caught exception: " + je._message);
			LOG_NOTICE("Uncomment below to force rebuild of scene xml on parse failure");
			abort();
			//		SceneXMLBuilder sceneXMLBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
			//									 boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
			//		SceneXMLParser sceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
			//									  boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));			
		}
		attemptParse = false;
	}
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
	
}
