/*
 *  SceneXMLBuilder.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _SCENEXMLBUILDER_H
#define	_SCENEXMLBUILDER_H

#include <boost/algorithm/string.hpp> // string splitting
#include <boost/regex.hpp> // regex
using boost::regex;
using boost::cmatch;
using boost::regex_search;
using boost::regex_match;

#include <algorithm>
#include <utility>
#include <map>
#include <string>
#include <vector>
#include <set>
using std::set;
using std::map;
using std::vector;
using std::string;

#include "ofxXmlSettings.h"
#include "goDirList.h"
#include "Logger.h"
#include "IXMLBuilder.h"
#include "JungleExceptions.h"
#include "Constants.h"
#include "SerializationUtils.h"
#include "AppModel.h"

#include "StringHelpers.h"

class SceneXMLBuilder : public IXMLBuilder {
public:
	SceneXMLBuilder(string dataPath, string xmlFile);
	
	
private:
	AppModel _builderAppModel;
	
	string			_xmlFile;
	string			_dataPath;

	goDirList		_moviesFileLister;
	goDirList		_assetsFileLister;


	void setupFileListers();
	void santiseFiles(); // lowercases all files ON DISK, any other checks?	
	void checkMovieAssets();
	void buildAppModel();
	void buildXML(); // builds actual xml from the info map

	
};

#endif