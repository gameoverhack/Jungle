/*
 *  SceneXMLBuilder.cpp
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "SceneXMLBuilder.h"

void listStuff(map<string, map<string, string> > m){
	map<string, map<string, string> >::iterator iter;
	iter = m.begin();	
	while (iter != m.end()) {
		printf("%s =| \n", (iter->first).c_str());
		map<string, string>::iterator iter2;
		iter2 = iter->second.begin();
		while(iter2 != iter->second.end()){
			printf("\t%s => %s\n", (iter2->first).c_str(), (iter2->second).c_str());
			iter2++;
		}
		iter++;
	}	
}

SceneXMLBuilder::SceneXMLBuilder(string dataPath, string xmlFile) : IXMLBuilder(xmlFile){
	LOG_NOTICE("Creating scene xml builder with path:" + dataPath + " and config:" + xmlFile);
	_dataPath = dataPath;

	setupLister();	// set up lister
	santiseFiles();	// make sure files are ok

	// scan the files we have, inserting any data about them we 
	// can discover from the files themselves (names, metadata, etc)
	scanFiles();	// build info map
//	printf("!!! Scanned files\n");
//	listStuff(_info);
	
	// Now check for any references to invalid sequences
	findAndFixInvalidSequences();

//	printf("!!! Fixed files\n");
//	listStuff(_info);

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
	string fullname;
	vector<string> substrings; // stores split string parts
	
	// iterate all files, discovering what we want to know about them
	for(int fileNum = 0; fileNum < _numFiles; fileNum++){
		fullname = _lister.getName(fileNum); // Used to work out types etc
		LOG_VERBOSE("Scanning file: " + fullname);

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
				if(regex_search(fullname, regex(".+?seq\\d+b$"))){
					// is b sequence
					fileInfo.insert(make_pair("type", "b"));
					fileInfo.insert(make_pair("nextSequence", kLAST_SEQUENCE_TOKEN)); // nothing comes after a b sequence in the scene
					fileInfo.insert(make_pair("interactivity", "none"));
				}
				else if(regex_search(fullname, regex(".+?seq\\d+a$"))){
					// is a sequence
					fileInfo.insert(make_pair("type", "a"));
					// seq01a -> seq01a_loop
					fileInfo.insert(make_pair("nextSequence", substrings[1]+"_loop"));
					fileInfo.insert(make_pair("interactivity", "victim"));

					// seq01a -> seq02b
					string resultSeqName = substrings[1]; // seq01a (ignores loop part which is substrings[2] if present
					resultSeqName = createNextSequenceString(resultSeqName); // seq01a to seq02a
					LOG_WARNING("MUST CONFIRM a_type sequence victimResult name is seq02b  (and not seq01b, probably isnt.)");
					resultSeqName[resultSeqName.length()-1] = 'b'; // make seq02a into seq02b TODO: this might be wrong name
					fileInfo.insert(make_pair("victimResult", resultSeqName));
				}
				else{
					LOG_ERROR("Found a file, but name does not describe type!: " + fullname);
					continue; // skip the rest
				}
				
				   
			}
		}

		// default info for all files
		fileInfo.insert(make_pair("dateCreated", _lister.getCreated(fileNum)));
		fileInfo.insert(make_pair("dateModified", _lister.getModified(fileNum)));
		fileInfo.insert(make_pair("size", ofToString(_lister.getSize(fileNum))));
		fileInfo.insert(make_pair("filename", _lister.getName(fileNum))); // might as well save this too.
		fileInfo.insert(make_pair("faked", "false"));

//		printf("%s\n", fullname.c_str());
//		map<string, string>::iterator iter;
//		iter=fileInfo.begin();
//		while(iter != fileInfo.end()){
//			printf("\t%s\t\t\t=\t%s\n", iter->first.c_str(), iter->second.c_str());
//			iter++;
//		}
		
		string key = fileInfo["scene"] + ":" + fileInfo["sequence"];
		if(fileInfo["type"] == "transform"){
			string transformName = fileInfo["filename"];
			vector<string> splitName; 
			boost::split(splitName, transformName, boost::is_any_of("_"));
			transformName = splitName[splitName.size() -1]; // just use atk1/vic etc
			transformName = transformName.substr(0, transformName.find_last_of(".")); // remove ext
			key = key + ":" + transformName;
			
		}
		// save info
		_info.insert(pair<string, map<string, string> >(key, fileInfo));
	}
	
	// Find last seq__a and set its following sequences to kLAST_SEQUENCE_TOKEN
	map<string, string> * lastA = NULL;
	for(map<string, map<string, string> >::iterator iter = _info.begin(); iter != _info.end(); iter++){
		map<string, string> & kvmap = (iter->second); // syntax convenience
		// check if the current kvmap is an A type, save it if it is
		if(kvmap["type"] == "a"){
			lastA = &kvmap;
		}
	}
	
	// check that we actually found one (almost certainly did)
	if(lastA == NULL){
		LOG_WARNING("Did not find last a type sequence to set to kLAST_SEQUENCE_TOKEN, are there any seq___a.mov files?!");		
	} else{
		// set as last in sequence
		(*lastA)["nextSequence"] = kLAST_SEQUENCE_TOKEN;
		(*lastA)["victimResult"] = kLAST_SEQUENCE_TOKEN;
	}
	
}

void SceneXMLBuilder::findAndFixInvalidSequences(){
	set<string> invalids;
	
	map<string, map<string, string> >::iterator iter = _info.begin();
	while(iter != _info.end()){
		map<string, string> & kvmap = (iter->second); // syntax convenience
		
		string scene = kvmap["scene"];
		string checkKey;
		LOG_VERBOSE("FindFixInvalid: " + iter->first);
		if(kvmap["type"] == "loop"){
			LOG_VERBOSE("\tLoop type");
			checkKey = scene + ":" + kvmap["attackerResult"];
			LOG_VERBOSE("\tfind "+checkKey);
			// do the find
			if(_info.find(checkKey) == _info.end()){
				LOG_VERBOSE("\t\tDid not find! Adding to invalids");
				// is invalid, so store in invalid vector
				invalids.insert(checkKey);
			}
			checkKey = scene + ":" +  kvmap["victimResult"];
			LOG_VERBOSE("\tfind "+checkKey);
			if(_info.find(checkKey) == _info.end()){
				LOG_VERBOSE("\t\tDid not find! Adding to invalids");
				// is invalid, so store in invalid vector
				invalids.insert(checkKey);
			}			
		}
		
		// Note, we ignore things when they are set to kLAST_SEQUENCE_TOKEN
		// this is because the last a sequence doesn't have a loop after it
		// We can be certain that the data in kvmap isn't corrupt because we've
		// just created it ourself.
		if(kvmap["type"] == "a"){
			LOG_VERBOSE("\ta seq type");
			// check victimResult and next sequence
			if(kvmap["victimResult"] != kLAST_SEQUENCE_TOKEN){
				checkKey = scene + ":" +  kvmap["victimResult"];
				LOG_VERBOSE("\tfind "+checkKey);
				// do the find
				if(_info.find(checkKey) == _info.end()){
					LOG_VERBOSE("\t\tDid not find! Adding to invalids");
					// is invalid, so store in invalid vector
					invalids.insert(checkKey);
				}
			}
			if(kvmap["nextSequence"] != kLAST_SEQUENCE_TOKEN){
				checkKey = scene + ":" +  kvmap["nextSequence"];
				LOG_VERBOSE("\tfind "+checkKey);
				if(_info.find(checkKey) == _info.end()){
					LOG_VERBOSE("\t\tDid not find! Adding to invalids");
					// is invalid, so store in invalid vector
					invalids.insert(checkKey);
				}
			}
		}
		if(kvmap["type"] == "b"){
			LOG_VERBOSE("\b seq type");
			if(kvmap["nextSequence"] != kLAST_SEQUENCE_TOKEN){
				LOG_WARNING(kvmap["name"] + " nextSequence was not __FINAL_SEQUENCE_, was " + kvmap["nextSequence"] + ", Fixing");
				kvmap["nextSequence"] = "__FINAL_SEQUENCE__";
			}
		}
		iter++;
	}
	
	// We now have a list of invalid sequence references,
	// so we'll try to construct some of these by cloning similar things.
	// we also have to clone the transforms for whatever we copy
	for(set<string>::iterator iter = invalids.begin(); iter != invalids.end(); iter++){
		string fullInvalidKey = *iter;
		vector<string> invalidKeyParts;
		boost::split(invalidKeyParts, fullInvalidKey, boost::is_any_of(":"));
		
		// set up the regex
		string regexPattern = invalidKeyParts[0] + ":seq\\d+";
		
		if(regex_search(fullInvalidKey, regex("b$"))){
			// b key
			regexPattern = regexPattern + "b$";
		}
		if(regex_search(fullInvalidKey, regex("a$"))){
			// a key
			regexPattern = regexPattern + "a$";
		}
		if(regex_search(fullInvalidKey, regex("_loop$"))){
			// loop key			
			regexPattern = regexPattern + ".+?_loop$$";
		}
		
		// stores our clone targets, 
		// invalids => valids
		map<string, string> matches;
		
		string match = "";
		
		// iterate over all the keys we have.
		for(map<string, map<string, string> >::iterator infoiter = _info.begin(); infoiter != _info.end(); infoiter++){
			// check this against our regex pattern
			if(regex_search(infoiter->first, regex(regexPattern))){
				// save our key (dont clone here because we shouldn't edit map during iteration.
				match = infoiter->first;
				break;
			}
		}
		
		// we might not find a possible fix
		if(match == ""){
			LOG_ERROR("Sequence " + fullInvalidKey + " does not exist, could not find target to clone for fake");
			throw JungleException("Could not fix missing sequence");
		}
		
		LOG_WARNING("Sequence " + fullInvalidKey + " does not exist, cloning from " + match);
		// found a fix, so clone (with transforms)
		// clone starting point
		_info[fullInvalidKey] = _info[match];
		_info[fullInvalidKey]["faked"] = "true"; // its fake now
		_info[fullInvalidKey]["sequence"] = invalidKeyParts[1]; // set to proper sequence name
		
		// set specifics, this is things nextSequences, etc.
		if(regex_search(fullInvalidKey, regex("b$"))){
			// nothing?
		}
		if(regex_search(fullInvalidKey, regex("a$"))){
			// a key
			// set nextSequence to proper loop 
			_info[fullInvalidKey]["nextSequence"] = invalidKeyParts[1] + "_loop";

			// set victimResult
			// seq01a -> seq02b
			string resultSeqName = invalidKeyParts[1]; // seq01a
			resultSeqName = createNextSequenceString(resultSeqName); // seq01a to seq02a
			LOG_WARNING("MUST CONFIRM a_type sequence victimResult name is seq02b  (and not seq01b, probably isnt.)");
			resultSeqName[resultSeqName.length()-1] = 'b'; // make seq02a into seq02b TODO: this might be wrong name
			_info[fullInvalidKey]["victimResult"] = resultSeqName;			
		}
		if(regex_search(fullInvalidKey, regex("_loop$"))){
			// loop key
			// set attackerResult
			// set victimResult
			LOG_WARNING("MUST CONFIRM loop to victim result sequence names (seq01a_loop to seq02b?)");
			vector<string> seqnameParts;
			boost::split(seqnameParts, invalidKeyParts[1], boost::is_any_of("_"));
			string resultSeqName = seqnameParts[0]; // seq01a (ignores loop part which is seqnameParts[1])
			resultSeqName = createNextSequenceString(resultSeqName); // seq01a to seq02a
			_info[fullInvalidKey]["attackerResult"] = resultSeqName;
			resultSeqName[resultSeqName.length()-1] = 'b'; // make seq02a into seq02b TODO: this might be wrong name
			_info[fullInvalidKey]["victimResult"] = resultSeqName;
			// set nextSequence to this sequence
			_info[fullInvalidKey]["nextSequence"] = invalidKeyParts[1];
		}
		
		// clone transforms if keys exist
		if(_info.find(match+":atk1") != _info.end()){
			// clone atk1
			_info[fullInvalidKey+":atk1"] = _info[match+":atk1"];
			_info[fullInvalidKey+":atk1"]["faked"] = "true";
			_info[fullInvalidKey+":atk1"]["sequence"] = invalidKeyParts[1];
		}
		if(_info.find(match+":atk2") != _info.end()){
			// clone atk2
			_info[fullInvalidKey+":atk2"] = _info[match+":atk2"];
			_info[fullInvalidKey+":atk2"]["faked"] = "true";
			_info[fullInvalidKey+":atk2"]["sequence"] = invalidKeyParts[1];
		}
		if(_info.find(match+":vic1") != _info.end()){
			// clone vic1
			_info[fullInvalidKey+":vic1"] = _info[match+":vic1"];
			_info[fullInvalidKey+":vic1"]["faked"] = "true";
			_info[fullInvalidKey+":vic1"]["sequence"] = invalidKeyParts[1];

		}
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
		LOG_VERBOSE("sequence key: " + fileInfo["scene"]+":"+fileInfo["sequence"] + "(" + fileInfo["type"] + ")");
		which = findSequenceWhich(fileInfo["scene"]+":"+fileInfo["sequence"]);

		// find the type of file we're dealing with
		if(fileInfo["type"] == "transform"){
			// push into node
			_xml.pushTag("sequence", which);

			// add the transform node and its info
			which = _xml.addTag("transform");
			// set attributes for this transform
			_xml.addAttribute("transform", "faked", fileInfo["faked"], which);
			_xml.addAttribute("transform", "filename", fileInfo["filename"], which);
			_xml.addAttribute("transform", "size", fileInfo["size"], which);
			_xml.addAttribute("transform", "dateCreated", fileInfo["dateCreated"], which);
			_xml.addAttribute("transform", "dateModified", fileInfo["dateModified"], which);
			
			_xml.popTag(); // pop sequence
			
		}
		else{
			_xml.addAttribute("sequence", "sequenceType", fileInfo["type"], which);
			if(fileInfo["type"] == "loop"){	
				// loop specific stuff
				// seq01a_loop V-> seq02b
				// seq01a_loop A-> seq02a
				_xml.addAttribute("sequence", "attackerResult", fileInfo["attackerResult"], which);
				_xml.addAttribute("sequence", "victimResult", fileInfo["victimResult"], which);
			}
			// every sequence has this stuff
			_xml.setAttribute("sequence", "interactivity", fileInfo["interactivity"], which);
			if(fileInfo["interactivity"] == "victim"){
				_xml.setAttribute("sequence", "victimResult", fileInfo["victimResult"], which);
			}
			// seq01a -> seq01a_loop
			// seq01a_loop N-> seq01a_loop
			_xml.addAttribute("sequence", "nextSequence", fileInfo["nextSequence"], which);
			_xml.addAttribute("sequence", "faked", fileInfo["faked"], which);
			_xml.addAttribute("sequence", "filename", fileInfo["filename"], which);
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
	boost::split(keys, seqKey, boost::is_any_of(":"));
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

