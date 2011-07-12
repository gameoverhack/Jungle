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

// Some rules about scenes
// -> = "leads to", N - no action, V - victim action, A - attacker action
// seq01a N-> seq01a_loop
// seq01a V-> seq02b
// seq01a_loop N-> seq01a_loop
// seq01a_loop V-> seq02b
// seq01a_loop A-> seq02a
// 
// by this,
// any seqXXb should be considered the final sequence in a scene

class SceneXMLBuilder {
public:
	SceneXMLBuilder(string dataPath, string xmlFile);
	bool santiseFiles(); // lowercases all files ON DISK, any other checks?
	bool scanFiles(); // creates info map of each file
	bool build(); // builds actual xml from the info map
	bool save();
private:
	ofxXmlSettings	_xml;

	// Yuck. Have to remember the "which" int that is associated with a 
	// scene name and sequence name(stored as "scene/sequence"), so we 
	// can get back at the right "which" numbers associated to those sequences
	// and scenes.
	// We have to do this because we might get things out of order and have to
	// get back to the correct, previously created, node
	// This is a member variable so we can access it in some helper functions
	// that push/pop around the xml graph
	map<string, int> _keyToXMLWhichMap;
	
	
	string			_xmlFile;
	string			_dataPath;

	goDirList		_lister;
	int				_numFiles;

	map<string, map<string, string> > _info; // filename, map

	void setupLister();
	string createNextSequenceString(string seq);
	int findSequenceWhich(string seqKey); // find 'which' value for key, see function for better explanation
	int findSceneWhich(string sceneKey);
	
};

#endif