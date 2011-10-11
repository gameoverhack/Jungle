/*
 *  SceneXMLParser.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */
#ifndef _H_SCENEXMLPARSER
#define _H_SCENEXMLPARSER

#include <stdio.h>
#include <stdlib.h>
#include <set>
#include <map>
#include <vector>
using std::vector;
using std::map;
using std::set;

#include <boost/lexical_cast.hpp> // i to string
#include <boost/algorithm/string.hpp> // string splitting
#include <boost/regex.hpp> // regex
using boost::regex;
using boost::cmatch;
using boost::regex_search;
using boost::regex_match;


#include "IXMLParser.h"
#include "BaseState.h"
#include "AppModel.h"
#include "AppDataTypes.h"
#include "SerializationUtils.h"
#include "goDirList.h"
#include "StringHelpers.h"

class SceneXMLParser : public IXMLParser {

public:

	SceneXMLParser(string dataPath, string xmlFile);

	bool compareTagAttribute(string xmltag, string attribute, string target, int which);
	void forceTagAttribute(string xmltag, string attribute, string target, int which);
	void forceTagAttribute(string xmltag, string attribute, int target, int which);
	void checkTagAttributesExist(string xmltag, vector<string> attributes, int which);
	void setupFileListers();
	void parseXML();


private:

	string					_dataPath;
	goDirList				_moviesFileLister;
	goDirList				_assetsFileLister;

};

#endif
