/*
 *  DataController.h
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_DATACONTROLLER
#define _H_DATACONTROLLER

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <string>
using std::string;

#include "tinyxml.h"

#include "Logger.h"
#include "AppModel.h"
#include "AppDataTypes.h"

class DataController {

public:
	DataController(string configFilePath);
//	~DataController();
	
	void loadAppProperties(string fs);
	void loadSceneData(string filePath);

	template<class vectorType>
	bool loadVector(string filePath, vector<vectorType> * vec);
	
private :
	
	TiXmlDocument *_xmldoc;
	
};


#endif