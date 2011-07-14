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

#include <set>
#include <map>
#include <vector>
using std::vector;
using std::map;
using std::set;

#include <boost/algorithm/string.hpp> // string splitting
#include <boost/regex.hpp> // regex
using boost::regex;
using boost::cmatch;
using boost::regex_search;
using boost::regex_match;

#include "IXMLParser.h"
#include "AppModel.h"
#include "AppDataTypes.h"
#include "VectorLoader.h"
#include "goDirList.h"
#include "goVideoPlayer.h"

class SceneXMLParser : public IXMLParser {
public:
	SceneXMLParser(string dataPath, string xmlFile);
	
	void parseXML();

	void validateMovieFileExistence();
	void validateMovieTransformLengths();
	void validateFileMetadata();

	void parseXML1();
	
private:
	string _dataPath;

	goDirList _dirLister;
	int _numFiles;
	map<string, int> _filenameToDirListerIDMap; // maps filenames to _dirLister.getX(ID)

	map<string, map<string, string> > _parsedData; // scene/sequence or scene/sequence/transformname, map of values for end type (seq or trans
	
	void setupDirLister();
	void populateDirListerIDMap();
	bool compareFileinfo(string filename, map<string, string> fileInfo);
	void checkTagAttributesExist(string xmltag, vector<string> attributes, int which);	
	int findFileIDForLister(string filename);

};

#endif