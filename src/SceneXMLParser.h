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
	
private:
	string _dataPath;

	goDirList _lister;
	int _numFiles;
	map<string, int> _filenameToListerIDMap; // maps filenames to _lister.getX(ID)

	void setupLister();
	void populateListerIDMap();
	bool compareFileinfo(string filename, map<string, string> fileInfo);
	
};

#endif