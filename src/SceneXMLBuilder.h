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
using std::map;
using std::vector;
using std::string;

#include "ofxXmlSettings.h"
#include "goDirList.h"
#include "Logger.h"


class SceneXMLBuilder {
public:
	SceneXMLBuilder(string dataPath, string xmlFile);
	bool santiseFiles(); // lowercases all files ON DISK, any other checks?
	bool scanFiles(); // creates info map of each file
	bool build(); // builds actual xml from the info map
	bool save();
	void setupLister();
	string createNextSequenceString(string seq);
private:
	ofxXmlSettings	_xml;

	string			_xmlFile;
	string			_dataPath;

	goDirList		_lister;
	int				_numFiles;

	map<string, map<string, string> > _info; // filename, map
	
};

#endif