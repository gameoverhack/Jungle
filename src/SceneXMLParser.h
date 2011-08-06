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

#include <stdlib.h>
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
#include "BaseState.h"
#include "AppModel.h"
#include "AppDataTypes.h"
#include "VectorUtils.h"
#include "goDirList.h"
#include "goVideoPlayer.h"

enum {
	kSCENEXMLPARSER_INIT,
	kSCENEXMLPARSER_SETUP,
	kSCENEXMLPARSER_PARSE_XML,
	kSCENEXMLPARSER_VALIDATING_MOVIE_FILE_EXISTENCE,
	kSCENEXMLPARSER_VALIDATING_FILE_METADATA,
	kSCENEXMLPARSER_VALIDATING_MOVIE_TRANSFORM_LENGTHS,
	kSCENEXMLPARSER_CREATING_APPMODEL,
	kSCENEXMLPARSER_FINISHED
};

class SceneXMLParser : public BaseState, public IXMLParser {

public:

	SceneXMLParser(string dataPath, string xmlFile);

	void registerStates();

	void parseXML();
	void update();

	//string getStateMessage();

	float getLoadingProgress();


private:
	
	vector<string>			_missingTransforms; // Holds missing transforms, instance var because we add to it in parseXML if there is no transform nodes for a sequence.

	string					_dataPath;

	float					_loadingProgress;
	string					_stateMessage;

	goDirList				_dirLister;
	int						_numFiles;
	map<string, int>		_filenameToDirListerIDMap; // maps filenames to _dirLister.getX(ID)

	// scene:sequence or scene:sequence:transformname, map of values for end type (seq or transfrorm)
	map<string, map<string, string> >	_parsedData;

	// convenience, save keys we've processed in some update methods
	// so we can re-enter and not re-process the same keys
	set<string>							_completedKeys;
	goVideoPlayer                       *_movie;

	// "doing" functions
	void setupDirLister();
	void populateDirListerIDMap();
	void validateMovieFileExistence();
	bool validateMovieTransformLengths();
	void validateFileMetadata();
	bool createAppModel();

	// Helper functions
	void listParsedData();
	bool compareFileinfo(string filename, map<string, string> fileInfo);
	void checkTagAttributesExist(string xmltag, vector<string> attributes, int which);
	int findFileIDForLister(string filename);
	string findFullFilePathForFilename(string filename);
	void updateLoadingState();
	int findSequenceNumberFromString(string name);

};

#endif
