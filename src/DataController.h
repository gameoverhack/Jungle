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

#include "tinyxml.h"

#include "Logger.h"
#include "AppModel.h"

class DataController {

public:
	DataController(string configFilePath);
//	~DataController();
	
	template<class vectorType>
	void loadVector(string filePath, vector<vectorType> & vec);
	
};


#endif