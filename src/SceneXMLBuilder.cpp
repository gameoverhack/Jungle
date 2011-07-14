/*
 *  SceneXMLBuilder.cpp
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "SceneXMLBuilder.h"


SceneXMLBuilder::SceneXMLBuilder(string dataPath, string xmlFile) : IXMLBuilder(xmlFile){
	LOG_NOTICE("Creating scene xml builder with path:" + dataPath + " and config:" + xmlFile);
	_dataPath = dataPath;

	setupLister();	// set up lister
	santiseFiles();	// make sure files are ok
	scanFiles();	// build info map
	buildXML();		// build xml
	if(!save()){	// save xml
		LOG_ERROR("Could not save XML");
		abort();
	}; 
}

// Set up file lister
// resets state so it can be called whenever you want to start fresh
void SceneXMLBuilder::setupLister(){
	_lister.reset();
	_lister.addDir(ofToDataPath(_dataPath, true));
	_lister.allowExt("mov");
	_lister.allowExt("MOV");
	_lister.allowExt("bin");
	_lister.allowExt("BIN");
	_numFiles = _lister.listDir(true);
	// quick error check
	if(_numFiles == 0){
		LOG_ERROR("Could not build scene config, goDirList reported 0 files found.");
		abort();
	}
	
}

// Makes sure our files are in good order
// This means all lowercase.
// (Anything else required?)
void SceneXMLBuilder::santiseFiles(){
	string filename;
	string filenameLower;
	string path, pathLower;
	
	for(int fileNum = 0; fileNum < _numFiles; fileNum++){
		// get the file name
		filename = _lister.getName(fileNum);
		filenameLower = filename; // have to copy, setting the output iterator in transform didnt work...
		std::transform(filenameLower.begin(), filenameLower.end(), filenameLower.begin(), ::tolower);
		path = _lister.getPath(fileNum);
		pathLower = path;
#ifdef TARGET_WIN32
		LOG_ERROR("SANTISE FILES DOES NOT HANDLE WINDOWS PATHS, can propbaly just use same code though. Requires testing");
		abort();
#endif
		pathLower.replace(pathLower.find_last_of(filename)-filename.length()+1, filename.length(), filenameLower);
		rename(path.c_str(), pathLower.c_str());
	}
	// have to relist dir
	setupLister();
}



// Scans files in directories, working out information relavent to them.
// inserting that information as key/value pairs (strings) into a map,
// then inserting that map into the _info member variable (keyed by filename
// Building the xml graph should be done entirely by just using the _info map
// THIS DOES NOT LOAD ANY EXTERNAL STRUCUTURES. Everything is just strings.
void SceneXMLBuilder::scanFiles(){
	// filename stuff
	string fullname;
	vector<string> substrings; // stores split string parts
	
	// iterate all files, discovering what we want to know about them
	for(int fileNum = 0; fileNum < _numFiles; fileNum++){
		fullname = _lister.getName(fileNum);
		// strip extension
		fullname = fullname.substr(0, fullname.find_last_of(".")); // remove ext
		
		map<string, string> fileInfo; // temp map to put props into.
		
		// split filename
		boost::split(substrings, fullname, boost::is_any_of("_")); 
		
		// save some basic stuff
		fileInfo.insert(make_pair("scene", substrings[0])); // scene name
		// need to know if its a loop file to get the right sequence name
		if(regex_search(fullname, regex("_loop"))){
			// cant set all props here because it might be movie or transform
			fileInfo.insert(make_pair("sequence", substrings[1] + "_loop")); // is loop
		}
		else{
			fileInfo.insert(make_pair("sequence", substrings[1])); // not loop
		}
		
		// find what kind of file it is and save specific details for those types
		if (regex_search (fullname, regex("_transform"))){
			// transform file
			fileInfo.insert(make_pair("type", "transform"));
			// what do we want to know about a transform file?
			// nothing? Have scene, sequence, file info...
		}else{
			// Not a transform, must be a movie, find out what type
			if(regex_search(fullname, regex("_loop"))){
				fileInfo.insert(make_pair("type", "loop"));
				// is a loop movie, set its vic,atk,nextSeq
				// seq01a_loop N-> seq01a_loop
				fileInfo.insert(make_pair("nextSequence", fileInfo.find("sequence")->second));
				// seq01a_loop A-> seq02a
				// seq01a_loop V-> seq02b
				string resultSeqName = substrings[1]; // seq01a (ignores loop part which is substrings[2] if present
				resultSeqName = createNextSequenceString(resultSeqName); // seq01a to seq02a
				fileInfo.insert(make_pair("attackerResult", resultSeqName));
				LOG_WARNING("MUST CONFIRM loop to victim result sequence names (seq01a_loop to seq02b?)");
				resultSeqName[resultSeqName.length()-1] = 'b'; // make seq02a into seq02b TODO: this might be wrong name
				fileInfo.insert(make_pair("victimResult", resultSeqName));
				fileInfo.insert(make_pair("interactivity", "both")); // both can play
			}
			else{
				// not a loop movie, just a or b
				// find out if its a b sequence
				if(regex_search(fullname, regex(".+seq\\d+b"))){
					// is b sequence
					fileInfo.insert(make_pair("type", "b"));
					fileInfo.insert(make_pair("nextSequence", "__FINAL_SEQUENCE__")); // nothing comes after a b sequence in the scene
					fileInfo.insert(make_pair("interactivity", "none"));
				}
				else{
					// is a sequence
					fileInfo.insert(make_pair("type", "a"));
					// seq01a -> seq01a_loop
					fileInfo.insert(make_pair("nextSequence", substrings[1]+"_loop"));
					fileInfo.insert(make_pair("interactivity", "victim"));
				}
				   
			}
		}

		// default info for all files
		fileInfo.insert(make_pair("dateCreated", _lister.getCreated(fileNum)));
		fileInfo.insert(make_pair("dateModified", _lister.getModified(fileNum)));
		fileInfo.insert(make_pair("size", ofToString(_lister.getSize(fileNum))));
		fileInfo.insert(make_pair("filename", _lister.getName(fileNum))); // might as well save this too.

//		printf("%s\n", fullname.c_str());
//		map<string, string>::iterator iter;
//		iter=fileInfo.begin();
//		while(iter != fileInfo.end()){
//			printf("\t%s\t\t\t=\t%s\n", iter->first.c_str(), iter->second.c_str());
//			iter++;
//		}

		// save info
		_info.insert(pair<string, map<string, string> >(fullname, fileInfo));
	}
}



void SceneXMLBuilder::buildXML(){
	map<string, string> fileInfo;
	
	int which; // used for ofxXmlSettings "which" params

	// set up xml basics
	_xml.addTag("config:scenes");

	// set document root
	// note both these are popped at end of method
	_xml.pushTag("config");
	_xml.pushTag("scenes");
	
	map<string, map<string, string> >::iterator iter = _info.begin();
	while(iter != _info.end()){
		fileInfo = (iter->second); // Copy here for nicer syntax (instead of using a pointer.)
		// find scene name and push into it
		string key = "scene";
		which = findSceneWhich(fileInfo[key]);
		_xml.pushTag("scene", which);
		// find sequence tag (don't push here cause adding sequences dones't require that
		which = findSequenceWhich(fileInfo["scene"]+"/"+fileInfo["sequence"]);

		// find the type of file we're dealing with
		if(fileInfo["type"] == "transform"){
			// push into node
			_xml.pushTag("sequence", which);

			// add the transform node and its info
			which = _xml.addTag("transform");
			// set attributes for this transform
			_xml.addAttribute("transform", "filename", fileInfo["filename"], which);
			
			_xml.addAttribute("transform", "size", fileInfo["size"], which);
			_xml.addAttribute("transform", "dateCreated", fileInfo["dateCreated"], which);
			_xml.addAttribute("transform", "dateModified", fileInfo["dateModified"], which);
			
			_xml.popTag(); // pop sequence
			
		}
		else{
			if(fileInfo["type"] == "loop"){	
				// loop specific stuff
				// seq01a_loop V-> seq02b
				// seq01a_loop A-> seq02a
				_xml.addAttribute("sequence", "attackerResult", fileInfo["attackerResult"], which);
				_xml.addAttribute("sequence", "victimResult", fileInfo["victimResult"], which);
			}
			// every sequence has this stuff
			_xml.setAttribute("sequence", "interactivity", fileInfo["interactivity"], which);
			// seq01a -> seq01a_loop
			// seq01a_loop N-> seq01a_loop
			_xml.addAttribute("sequence", "nextSequence", fileInfo["nextSequence"], which);
			_xml.addAttribute("sequence", "size", fileInfo["size"], which);
			_xml.addAttribute("sequence", "dateCreated", fileInfo["dateCreated"], which);
			_xml.addAttribute("sequence", "dateModified", fileInfo["dateModified"], which);
		} 
		_xml.popTag(); // pop scene
		iter++; // next file
	}
	_xml.popTag(); // pop scenes
	_xml.popTag(); // pop config
}


// Increments the sequence number in a sequence name
// eg: seq01a -> seq02a
string SceneXMLBuilder::createNextSequenceString(string seq){
	int seqNum;
	char seqNumString[20];
	char formatString[20];
	string matchstring;
	
	
	cmatch match;
	string ret = seq;
	
	// pull out the sequence number from the sequence name, so we can increment it
	if(regex_match(seq.c_str(), match, regex("seq(\\d+)a"))){
		matchstring = string(match[1].first, match[1].second);
		sscanf(matchstring.c_str(), "%d", &seqNum); // find int
		seqNum++; // inc int
		// get correct width in format string
		snprintf(formatString, sizeof(formatString), "%%0%dd", (int)matchstring.length());
		snprintf(seqNumString, sizeof(seqNumString), formatString, seqNum);
		ret.replace(ret.find_last_of(matchstring)-matchstring.length()+1,
					matchstring.length(),
					seqNumString);
	}
	else{
		LOG_ERROR("Could not find int to increment");
		abort();
	}
	return ret;
}

// Find the 'which' value for a scene and sequence key
// These names should be in the format "scene/sequence"
// if the sequence part does not exist, it is created.
// NOTE:	This expects to have been pushTag()'d into a scene node already
//			IT DOES NOT CREATE THE SCENE NODE IF IT DOESN'T EXIST
int SceneXMLBuilder::findSequenceWhich(string seqKey){
	int which;
	vector<string> keys;
	boost::split(keys, seqKey, boost::is_any_of("/"));
	if(_keyToXMLWhichMap.find(seqKey) == _keyToXMLWhichMap.end()){
		// Doesn't exist, so create a sequence with the right name
		which = _xml.addTag("sequence");
		// NOTE: this is only a partial creation, does not set all attributes
		_xml.setAttribute("sequence", "name", keys[1], which);
		// add info to map
		_keyToXMLWhichMap.insert(make_pair(seqKey, which));
	}
	// Find which sequence to push into
	which = _keyToXMLWhichMap.find(seqKey)->second;
	return which;
}

// Find the 'which' value for a scene key
// creates the scene xml node if it does not exist and adds the which value
// to the name to which map.
int SceneXMLBuilder::findSceneWhich(string sceneKey){
	int which;
	// does the scene already exist?
	if(_keyToXMLWhichMap.find(sceneKey) == _keyToXMLWhichMap.end()){
		// doesn't exist, add it
		which = _xml.addTag("scene");
		// NOTE partial setup, name only.
		_xml.addAttribute("scene", "name", sceneKey, which);
		// Add to info map
		_keyToXMLWhichMap.insert(make_pair(sceneKey, which));
	}
	// get "which" scene tag for this scene name
	which = _keyToXMLWhichMap.find(sceneKey)->second;
	return which;
}

