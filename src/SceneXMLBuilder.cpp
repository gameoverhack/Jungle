/*
 *  SceneXMLBuilder.cpp
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "SceneXMLBuilder.h"


SceneXMLBuilder::SceneXMLBuilder(string dataPath, string xmlFile){
	LOG_NOTICE("Creating scene xml builder with " + dataPath + " and " + xmlFile);
	_xmlFile = xmlFile;
	_dataPath = dataPath;

	setupLister();
}

void SceneXMLBuilder::setupLister(){
	_lister.reset();
	printf("%s\n", ofToDataPath(_dataPath, true).c_str());
	_lister.addDir(ofToDataPath(_dataPath, true));
	_lister.allowExt("mov");
	_lister.allowExt("MOV");
	_lister.allowExt("bin");
	_lister.allowExt("BIN");
	_numFiles = _lister.listDir(true);
}

bool SceneXMLBuilder::santiseFiles(){
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
		LOG_ERROR("SANTISE FILES DOES NOT HANDLE WINDOWS PATHS, can propbaly just use same code though.");
		abort();
#endif
		pathLower.replace(pathLower.find_last_of(filename)-filename.length()+1, filename.length(), filenameLower);
		rename(path.c_str(), pathLower.c_str());
	}
	// have to relist dir
	setupLister();
	return true;
}

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
		printf("FOrmatstring: '%s'\n", formatString);
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

bool SceneXMLBuilder::scanFiles(){

	// filename stuff
	string fullname;
	vector<string> substrings; // stores split string parts
	
	// regex patterns
	// matches (any scene)_seq(any digits)_(anything)_transform.bin
//	regex transformFilenamePattern(".+_seq\\d+[a|b]_.+_transform\\.bin");

	//	regex transformFilenamePattern(".+_seq\\d+[a|b]_.+_transform\\.bin", boost::regex_constants::icase|boost::regex_constants::perl);
//	// matches (any scene)_seq(any digits)(a or b)_loop.mov
//	regex loopFilenamePattern(".+_seq\\d+[a|b]_loop\\.mov", boost::regex_constants::icase|boost::regex_constants::perl);
//	// matches (any scene)_seq(any digits)a.mov
//	regex aFilenamePattern(".+_seq\\d+a\\.mov", boost::regex_constants::icase|boost::regex_constants::perl);
//	// matches (any scene)_seq(any digits)b.mov
//	regex bFilenamePattern(".+_seq\\d+b\\.mov", boost::regex_constants::icase|boost::regex_constants::perl);

	// add and list the dir
	
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
				if(regex_search(fullname, regex(".+seq\\db"))){
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
//
//			// find out if its a "b" sequence
//			if(regex_search(filename, regex(".+seq\\db\\.mov"))){
//				// is a b sequence
//				fileInfo.insert(make_pair("aorb", "b"));
//			}
//			else{
//				// not b, so is a
//				fileInfo.insert(make_pair("aorb", "a");
//			}
							
		}

		// default info for all files
		fileInfo.insert(make_pair("dateCreated", _lister.getCreated(fileNum)));
		fileInfo.insert(make_pair("dateModified", _lister.getModified(fileNum)));
		fileInfo.insert(make_pair("size", ofToString(_lister.getSize(fileNum))));
		fileInfo.insert(make_pair("filename", _lister.getName(fileNum))); // might as well save this too.

		printf("%s\n", fullname.c_str());
		map<string, string>::iterator iter;
		iter=fileInfo.begin();
		while(iter != fileInfo.end()){
			printf("\t%s\t\t\t=\t%s\n", iter->first.c_str(), iter->second.c_str());
			iter++;
		}
		// save
		_info.insert(pair<string, map<string, string> >(fullname, fileInfo));
	}
}

bool SceneXMLBuilder::build(){
	goDirList lister;
	int _numFiles;
	map<string, int> filenameToListPosMap;

	// Yuck. have to remember the "which" int that is associated with a 
	// scene name, so we can get back at the right ones to insert details
	// for the scene. Techically, the lister might always return the files
	// correctly grouped, but theres no guarentee to the structure
	map<string, int> nameToXMLWhichMap;
	int which; // used for ofxXmlSettings "which" params


	vector<string> substrings; // stores split string parts


	// quick error check
	if(_numFiles == 0){
		LOG_ERROR("Could not build scene config, goDirList reported 0 files.");
		abort();
	}

	// set up xml basics
	_xml.addTag("config:scenes");
	_xml.pushTag("config");
	_xml.pushTag("scenes"); // set doc root

	// Iterate the files 
	for(int fileNumber = 0; fileNumber < _numFiles; fileNumber++){
		string filename = lister.getName(fileNumber);
		filename = filename.substr(0, filename.find_last_of(".")); // remove ext
		boost::split(substrings, filename, boost::is_any_of("_")); // split filename
		
		// first substring is the scene name, check if its existant
		if(nameToXMLWhichMap.find(substrings[0]) == nameToXMLWhichMap.end()){
			// doesn't exist, add it
			which = _xml.addTag("scene");
			_xml.addAttribute("scene", "name", substrings[0], which);
			// Add to info map
			nameToXMLWhichMap.insert(pair<string, int>(substrings[0], which));			
		}
		
		// get "which" scene tag for this scene name
		which = nameToXMLWhichMap.find(substrings[0])->second;
		// push into scene
		_xml.pushTag("scene", which);
		
		// add sequence to scene
		
		// transform file for a sequence
		if(filename.find("transform") !=string::npos){
			// does the sequence exist?
			string mapName = substrings[1];
			if(filename.find("loop") != string::npos){
				// loop file, so append _loop to mapname
				mapName = substrings[1] + "_loop";
			}
			
			if(nameToXMLWhichMap.find(mapName) == nameToXMLWhichMap.end()){
				// doesn't exist, so create a sequence with the right name
				which = _xml.addTag("sequence");
				_xml.setAttribute("sequence", "name", mapName, which);
				// NOTE: this is only a partial creation, does not set all attributes
				nameToXMLWhichMap.insert(pair<string, int>(mapName, which));
			}
			// Find which sequence to push into
			which = nameToXMLWhichMap.find(mapName)->second;
			
			// push into sequence
			_xml.pushTag("sequence", which);
			
			// insert transform stuff
			which = _xml.addTag("transform");
			// set attributes for this transform
			_xml.addAttribute("transform", "filename", lister.getName(fileNumber), which);
			
			_xml.addAttribute("transform", "size", lister.getSize(fileNumber), which);
			_xml.addAttribute("transform", "dateCreated", lister.getCreated(fileNumber), which);
			_xml.addAttribute("transform", "dateModified", lister.getModified(fileNumber), which);
			
			_xml.popTag(); // pop sequence			
		}
		// loop file for a sequence
		else if(filename.find("loop") != string::npos){
			// does the sequence exist, it might not because we might get a transform
			// before we get a movie, in which case we'll have inserted a basic
			// sequence tag
			string mapName = substrings[1] + "_loop";
			
			if(nameToXMLWhichMap.find(mapName) == nameToXMLWhichMap.end()){
				// doesn't exist, so create a sequence with the right name
				which = _xml.addTag("sequence");
				_xml.setAttribute("sequence", "name", mapName, which);
				// NOTE: this is only a partial creation, does not set all attributes
				nameToXMLWhichMap.insert(pair<string, int>(mapName, which));
			}
			which = nameToXMLWhichMap.find(mapName)->second;
			
			// add the sequence attributes
			_xml.setAttribute("sequence", "name", mapName, which);
			_xml.setAttribute("sequence", "interactive", "both", which);
			
			// seq01a_loop N-> seq01a_loop
			_xml.addAttribute("sequence", "nextSequence", mapName, which);
			
			
			// seq01a_loop V-> seq02b
			// seq01a_loop A-> seq02a
			
			int seqNum;
			// pull out the sequence number from the sequence name, so we can increment it
			if(sscanf(substrings[1].c_str(), "seq%da_loop", &seqNum) != 1){
				LOG_ERROR("Could not sscanf sequence number from sequence name "+substrings[1]);
				abort();
			}
			seqNum++;
			char seqNumString[10]; // should never have more than 10 digits...
			snprintf(seqNumString, sizeof(seqNumString), "%02d", seqNum);
			// create a new sequence name
			string newName = "seq"+string(seqNumString)+"a";
			_xml.addAttribute("sequence", "attackerResult", newName, which);
			newName = "seq"+string(seqNumString)+"b";
			_xml.addAttribute("sequence", "victimResult", newName, which);
			
			_xml.addAttribute("sequence", "size", lister.getSize(fileNumber), which);
			_xml.addAttribute("sequence", "dateCreated", lister.getCreated(fileNumber), which);
			_xml.addAttribute("sequence", "dateModified", lister.getModified(fileNumber), which);
		}
		else{
			// Not a loop, not a transform, is a regular sequence movie
			string mapName = substrings[1];
			
			if(nameToXMLWhichMap.find(mapName) == nameToXMLWhichMap.end()){
				// doesn't exist, so create a sequence with the right name
				which = _xml.addTag("sequence");
				_xml.setAttribute("sequence", "name", mapName, which);
				// NOTE: this is only a partial creation, does not set all attributes
				nameToXMLWhichMap.insert(pair<string, int>(mapName, which));
			}
			which = nameToXMLWhichMap.find(mapName)->second;
			
			// add the sequence attributes
			_xml.setAttribute("sequence", "name", substrings[1], which); // just second part
			_xml.setAttribute("sequence", "interactive", "victim", which); // only victim can interact			
			
			// seq01a N-> seq01a_loop
			_xml.addAttribute("sequence", "nextSequence", substrings[1]+"_loop", which); // seq01a -> seq01a_loop
			
			
			// seq01a V-> seq02b
			int seqNum;
			// pull out the sequence number from the sequence name, so we can increment it
			if(sscanf(substrings[1].c_str(), "seq%da_loop", &seqNum) != 1){
				LOG_ERROR("Could not sscanf sequence number from sequence name "+substrings[1]);
				abort();
			}
			seqNum++;
			char seqNumString[10]; // should never have more than 10 digits...
			snprintf(seqNumString, sizeof(seqNumString), "%02d", seqNum);
			// create a new sequence name
			string newName = "seq"+string(seqNumString)+"b";
			_xml.addAttribute("sequence", "victimResult", newName, which); // seq01a -> seq01b
			
			_xml.addAttribute("sequence", "size", lister.getSize(fileNumber), which);
			_xml.addAttribute("sequence", "dateCreated", lister.getCreated(fileNumber), which);
			_xml.addAttribute("sequence", "dateModified", lister.getModified(fileNumber), which);
		}
		_xml.popTag(); // scene pop 
	}

	// re iterate over the xml
	string seqName = "";
	int scanfInt;
	for(int sceneNum = 0; sceneNum < _xml.getNumTags("scenes"); sceneNum++){
		_xml.pushTag("scenes", sceneNum);
		for(int seqNum = 0; seqNum < _xml.getNumTags("sequence"); seqNum++){
			// check if name is a b, set nextSequence to __FINAL__SEQUENCE__
			seqName = _xml.getAttribute("sequence", "name", seqName, seqNum);
			if(seqName[seqName.length()-1] == 'b'){ // brittle, but should hold up. Don't process _loop
				_xml.setAttribute("sequence", "nextSequence", "__FINAL_SEQUENCE__", seqNum);
			}
			// mark last sequence a and b in scene as final sequence
			// nothing else to do but assume finals are at the end of the file list
			// and hence at the end of the xml.
		}
		_xml.popTag();
	}

	// set last sequences to __FINAL_SEQUENCE__
	// set all sequences with b in them to __FINAL_SEQUENCE__
	// TODO
}

bool SceneXMLBuilder::save(){
	if(_xml.saveFile(_xmlFile+"_temp.xml")){
		// remove the first file
		remove(ofToDataPath(_xmlFile, true).c_str());
		// rename temp to final file
		rename(ofToDataPath(_xmlFile+"_temp.xml", true).c_str(), ofToDataPath(_xmlFile, true).c_str());
		return true;
	}
	else{
		LOG_ERROR("Could not save properties to xml. File error?");
		return false;
	}
}
