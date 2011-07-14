/*
 *  SceneXMLParser.cpp
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "SceneXMLParser.h"
#include "JungleExceptions.h"

SceneXMLParser::SceneXMLParser(string dataPath, string xmlFile) : IXMLParser(xmlFile){
	_dataPath = dataPath;
	
	setupDirLister(); // set up file lists;
	populateDirListerIDMap();
	load(); // load xml
	parseXML(); // make map from xml
	
	// Check that the map contains valid files (replace with temp fakes if it doesnt)
	// throws JungleException if some files can't be fixed that should cause a crash since we can't go on at all.
	// OPTIONALLY:	We could look at "validFile" key when we create the model and not add that sequence,
	//				that way we still run, but 
	try{
		validateMovieFileExistence();
	}
	catch (JungleException je) {
		// Some files could not be fixed, 
		// we could look at a property "ignoreInvalidSequences" and just not add
		// those to the appModel. That way we run still, and when we get to changing
		// to a sequence that has no file (and couldn't be fixed) 
		// we'll just get the log error message 'no sequence seqXXa'
		throw je; // for now we'll just consider it a hard fail and throw up.
	}
	
	try{
		validateFileMetadata();
	}
	catch(vector<string> vec){
		string message = "";
		for(vector<string>::iterator iter = vec.begin(); iter != vec.end(); iter++){
			message = *iter + ", " + message;
		}
		message[message.length()-1] = ' ';
		message = "Metadata xml vs hdd mismatch for files: " + message;
		LOG_ERROR(message);
		throw MetadataMismatchException(message);
	}
	
	try{
		validateMovieTransformLengths();
	}
	catch (vector<string> vec) {
		// means some files were missing, probably just pass this back up.
		string message = "";
		for(vector<string>::iterator iter = vec.begin(); iter != vec.end(); iter++){
			message = *iter + ", " + message;
		}
		message[message.length()-1] = ' ';
		LOG_ERROR("Require reanalysis/creation of transform files: " + message);
		throw vec;
	}
	
}


// Expects to be pushed/popped into the correct level
void SceneXMLParser::checkTagAttributesExist(string xmltag, vector<string> attributes, int which){
	string message = "";
	for(vector<string>::iterator iter = attributes.begin(); iter != attributes.end(); iter++){
		if(!_xml.attributeExists(xmltag, *iter, which)){
			message = message + *iter + ","; // build list of missing
		}
	}
	
	// if we've added to message, it has a length
	if(message.length() != 0){
		// remove last comma
		message[message.length()-1] = ' ';
		message = "Tag " + xmltag + " missing attributes " + message;
		// throw exception
		LOG_ERROR(message);
		throw JungleException(message);
	}
}

// Parse xml into a map structure which we can then build the model from in populateAppModel
void SceneXMLParser::parseXML(){
	string sceneName, sequenceName, transformName, mapKey;
	string stringType = "default string, if you see this, getAttribute failed";
	vector<string> attributesToCheck;
	
	// Set up _xml root node
	if(!_xml.tagExists("config")){ // quick check
		LOG_ERROR("No config node in xml");
		throw JungleException("No config and scenes node in xml");
	}
	_xml.pushTag("config");
	LOG_VERBOSE("Parsing scene data");
	if(!_xml.tagExists("scenes")){ // quick check
		LOG_ERROR("No scenes node in xml!");
		throw JungleException("No scenes node in xml");
	}
	_xml.pushTag("scenes");
	// document root acceptable
	
	// sanity check
	if(_xml.getNumTags("scene") == 0){
		LOG_ERROR("No scene nodes in xml");
		throw JungleException("No scene nodes to xml");
	}
	
	// push into each scene node
	for(int sceneNum = 0; sceneNum < _xml.getNumTags("scene"); sceneNum++){

		// check attributes
		attributesToCheck.clear();
		attributesToCheck.push_back("name");
		checkTagAttributesExist("scene", attributesToCheck, sceneNum);

		// Remember the name
		sceneName = _xml.getAttribute("scene", "name",stringType, sceneNum);
		LOG_VERBOSE("Currently xml=>map scene: " + sceneName);

		mapKey = sceneName;
		_parsedData[mapKey]["name"] = sceneName;
		_parsedData[mapKey]["type"] = "scene";
		_parsedData[mapKey]["positionInXML"] = ofToString(sceneNum);
		
		// set new root
		_xml.pushTag("scene", sceneNum);
		
		// sanity check
		if(_xml.getNumTags("sequence") == 0){
			LOG_ERROR("No sequence nodes for scene in xml");
			throw JungleException("No sequence nodes for scene in xml");
		}
		// iterate over all the sequences
		for(int seqNum = 0; seqNum < _xml.getNumTags("sequence"); seqNum++){

			// check attributes
			attributesToCheck.clear();
			attributesToCheck.push_back("name");
			attributesToCheck.push_back("size");
			attributesToCheck.push_back("dateModified");
			attributesToCheck.push_back("dateCreated");
			attributesToCheck.push_back("interactivity");
			checkTagAttributesExist("sequence", attributesToCheck, seqNum);			
						
			// Remember the name
			sequenceName = _xml.getAttribute("sequence", "name", stringType, seqNum);
			LOG_VERBOSE("Currently xml=>map sequence: " + sequenceName);
			
			// construct key for parsed data map, rebuild instead of just append to be clearer
			mapKey = sceneName+":"+sequenceName;
			
			// save some details
			_parsedData[mapKey]["name"] = sequenceName;
			_parsedData[mapKey]["type"] = "sequence";
			
			// save movie file name
			_parsedData[mapKey]["filename"] = sceneName+"_"+sequenceName+".mov";
			_parsedData[mapKey]["validFile"] = "unvalidated";
			
			// save all the file meta data
			_parsedData[mapKey]["size"] = _xml.getAttribute("sequence", "size", stringType, seqNum);
			_parsedData[mapKey]["dateCreated"] = _xml.getAttribute("sequence", "dateCreated", stringType, seqNum);															
			_parsedData[mapKey]["dateModified"] = _xml.getAttribute("sequence", "dateModified", stringType, seqNum);															

			// save interactivity info
			_parsedData[mapKey]["interactivity"] = _xml.getAttribute("sequence", "interactivity", stringType, seqNum);
			
			// find transforms for this sequence
			// Push into this sequence so we insert into the right one.
			_xml.pushTag("sequence", seqNum);
			// sanity check
			if(_xml.getNumTags("transform") == 0){
				LOG_ERROR("No transform nodes for sequence in xml");
				throw JungleException("No transform nodes for sequence in xml");
			}
			for(int transNum = 0; transNum < _xml.getNumTags("transform"); transNum++){
				// check attributes
				attributesToCheck.clear();
				attributesToCheck.push_back("filename");
				attributesToCheck.push_back("size");
				attributesToCheck.push_back("dateModified");
				attributesToCheck.push_back("dateCreated");
				checkTagAttributesExist("transform", attributesToCheck, transNum);
				
				transformName = _xml.getAttribute("transform", "filename", stringType, transNum);
				vector<string> splitName; 
				boost::split(splitName, transformName, boost::is_any_of("_"));
				transformName = splitName[splitName.size() -1]; // just use atk1/vic etc
				transformName = transformName.substr(0, transformName.find_last_of(".")); // remove ext
				
				// set up map key
				mapKey = sceneName+":"+sequenceName+":"+transformName;

				_parsedData[mapKey]["name"] = transformName;
				_parsedData[mapKey]["type"] = "transform";
				_parsedData[mapKey]["filename"] = _xml.getAttribute("transform", "filename", stringType, transNum);
				_parsedData[mapKey]["size"] = _xml.getAttribute("transform", "size", stringType, transNum);
				_parsedData[mapKey]["dateCreated"] = _xml.getAttribute("transform", "dateCreated", stringType, transNum);															
				_parsedData[mapKey]["dateModified"] = _xml.getAttribute("transform", "dateModified", stringType, transNum);															
			}
			_xml.popTag(); // pop sequence
		}
		_xml.popTag(); // pop scene
	}
	

	_xml.popTag(); // scenes pop
	_xml.popTag(); // config pop
}

// checks that files referenced by sequences exist,
// if they don't we replace them with a fake and flag the data is faked
// throws JungleException if some files can't be fixed.
void SceneXMLParser::validateMovieFileExistence(){
	int fileID;
	string fullFilePath;
	
	vector<string> invalids;
	
	// Check for any non existing files
	map<string, map<string, string> >::iterator parsedDataIter;
	for(parsedDataIter = _parsedData.begin(); parsedDataIter != _parsedData.end(); parsedDataIter++){
		map<string, string> & kvmap = (parsedDataIter->second); // syntax convenience
		if(kvmap["type"] == "scene" || kvmap["type"] == "transform"){
			continue;
		}
		if(kvmap.find("filename") == kvmap.end() ){
			throw JungleException("Required one of kvmap keys filename missing for " + parsedDataIter->first);
		}
		// get the file
		try{
			fileID = findFileIDForLister(kvmap["filename"]); // throws exception on file not found
			kvmap["validFile"] = "valid";
		}
		catch (JungleException je) {
			// caught exception, meaning no file found, 
			// save to check later. We check later so we can be sure we 
			// assign the fake movie from a valid sequence and not just
			// make this one using another invalid file 
			// (incase this is the first and we haven't checked any othersr)
			kvmap["validFile"] = "invalid";
			invalids.push_back(parsedDataIter->first);
		}
	}
	
	// Now actually fix up invalid files.
	for (vector<string>::iterator iter = invalids.begin(); iter != invalids.end(); iter++) {
		// check for keys that are similar to the key we have
		LOG_WARNING("Fixing non existant movie for " + *iter);
		
		// build our regex
		vector<string> invalidKeySplit;
		boost::split(invalidKeySplit, *iter, boost::is_any_of(":"));
		string regexPattern = invalidKeySplit[0]+":seq\\d+"; // our scene, any sequence
		if(regex_search(invalidKeySplit[1], regex("_loop"))){
			regexPattern = regexPattern + ".+?_loop$"; // we are a loop, so find another loop, must be at end of key
		}
		else{
			regexPattern = regexPattern + "$"; // must be at end of key
		}
		regex similarTo = regex(regexPattern);
		LOG_VERBOSE("Searching for key similar to " + regexPattern);
		
		// iterate over parsed data, comparing keys
		for(parsedDataIter = _parsedData.begin(); parsedDataIter != _parsedData.end(); parsedDataIter++){
			if(regex_search(parsedDataIter->first, similarTo)){
				// found a similar key, copy values, make sure it is valid
				string bad = *iter;
				string good = parsedDataIter->first;
				
				if(_parsedData[good]["validFile"] != "valid"){
					continue; // similar key, but this keys files are invalid anyway, keep looking.
				}
				LOG_WARNING("Copying values: " + good + " => " + bad);
				
				// copy movie stuff
				_parsedData[bad]["filename"] = _parsedData[good]["filename"];
				_parsedData[bad]["size"] = _parsedData[good]["size"];
				_parsedData[bad]["dateModified"] = _parsedData[good]["dateModified"];
				_parsedData[bad]["dateCreated"] = _parsedData[good]["dateCreated"];
				
				// copy transform stuff
				// remove the ones for bad
				_parsedData.erase(bad+":vic1");
				_parsedData.erase(bad+":atk1");
				_parsedData.erase(bad+":atk2");
				// recreate with good
				if(_parsedData.find(good+":vic1") != _parsedData.end()){
					_parsedData[bad+":vic1"] = _parsedData[good+":vic1"];
				}
				if(_parsedData.find(good+":atk1") != _parsedData.end()){
					_parsedData[bad+":atk1"] = _parsedData[good+":atk1"];
				}
				if(_parsedData.find(good+":atk2") != _parsedData.end()){
					_parsedData[bad+":atk2"] = _parsedData[good+":atk2"];
				}
				
				// flag the data as faked
				_parsedData[bad]["validFile"] = "fake";
				// would remove "bad" from list of invalids now but shouldn't modify while iterating.
				break; // stop looking
			}
		}
	}
	
	// once again, iterate over the invalids, check for any that are still "invalid"
	// and add them to a new vector which we'll throw as an exception
	vector<string> couldNotFix;
	for (vector<string>::iterator iter = invalids.begin(); iter != invalids.end(); iter++) {
		if(_parsedData[*iter]["validFile"] == "invalid"){
			couldNotFix.push_back(*iter);
		}
	}
	if(couldNotFix.size() != 0){
		string message = "";
		for(vector<string>::iterator iter = couldNotFix.begin(); iter != couldNotFix.end(); iter++){
			message = message + *iter + ", ";
		}
		message[message.length()-1] = ' ';
		message = "No movies for these keys " + message + ", attempted fix but failed. " 
				+ "Either there was no other sequences for scene or all other sequences were invalid too.";
		throw JungleException(message);
	}
	
}

// Checks that dateCreated, dateModified and size are the same on disk and in xml
// throws vector<string> containing all broken files if any present
// throws JungleException if a required key is missing.
void SceneXMLParser::validateFileMetadata(){
	int fileID;
	set<string> invalidFiles; // use set first, don't want duplicates
	vector<string> invalidFilesVector; // convert to vector on return.
	map<string, map<string, string> >::iterator parsedDataIter;
	for(parsedDataIter = _parsedData.begin(); parsedDataIter != _parsedData.end(); parsedDataIter++){
		map<string, string> & kvmap = (parsedDataIter->second); // syntax convenience
		if(kvmap["type"] == "scene"){
			continue;
		}
		// check that the map has the right keys
		if(kvmap.find("size") == kvmap.end() ||
		   kvmap.find("dateModified") == kvmap.end() ||
		   kvmap.find("dateCreated") == kvmap.end() ||
		   kvmap.find("filename") == kvmap.end() ){
			throw JungleException("Required one of kvmap keys filename, size, dateModified or dateCreated missing for " + parsedDataIter->first);
		}
		
		fileID = findFileIDForLister(kvmap["filename"]);
		
		
		if(_dirLister.getCreated(fileID) != kvmap["dateCreated"]){
			LOG_WARNING("dateCreated mismatch on " + kvmap["filename"] + "("+_dirLister.getCreated(fileID)
						+ " hdd vs " + kvmap["dateCreated"]+" xml)");
			invalidFiles.insert(kvmap["filename"]);
			
		}
		if(_dirLister.getModified(fileID) != kvmap["dateModified"]){
			LOG_WARNING("dateModified mismatch on " + kvmap["filename"] + "("+_dirLister.getCreated(fileID)
						+ " hdd vs " + kvmap["dateModified"]+" xml)");
			invalidFiles.insert(kvmap["filename"]);
		}
		if(ofToString(_dirLister.getSize(fileID)) != kvmap["size"]){
			LOG_WARNING("size mismatch on " + kvmap["filename"] + "("+ofToString(_dirLister.getSize(fileID))
						+ " hdd vs " + kvmap["size"]+" xml)");
			invalidFiles.insert(kvmap["filename"]);
		}
	}
	
	if(invalidFiles.size() != 0){
		// have invalids, make vector of them
		for(set<string>::iterator iter = invalidFiles.begin(); iter != invalidFiles.end(); iter++){
			invalidFilesVector.push_back(*iter);
		}
		throw invalidFilesVector;
	}
}

// checks that transform lengths are the same as their movies
// might throw a vector<string> as an exception with the list of 
// transforms which need to be recompiled
void SceneXMLParser::validateMovieTransformLengths(){
	goVideoPlayer *movie = NULL;
	vector<CamTransform> transform;

	int fileID;
	string fullFilePath;
	
	vector<string> transformFilesRequired;
	
	// iterate over the whole parsed data map
	map<string, map<string, string> >::iterator parsedDataIter;
	for(parsedDataIter = _parsedData.begin(); parsedDataIter != _parsedData.end(); parsedDataIter++){
		map<string, string> & kvmap = (parsedDataIter->second); // syntax convenience
		LOG_VERBOSE("Checking files for " + parsedDataIter->first);
		
		// make sure filename is exists
		if(kvmap["type"] == "sequence" || kvmap["type"] == "transform"){			
			if(kvmap.find("filename") == kvmap.end()){
				LOG_ERROR("No filename attribute in kvmap for " + kvmap["name"]);
				throw JungleException("No filename attribute in kvmap for " + parsedDataIter->first);
			}
		}
			
		// handle each type (sequence, transfom orscene)
		if(kvmap["type"] == "sequence"){
			try {
				fileID = findFileIDForLister(kvmap["filename"]);
				fullFilePath = _dirLister.getPath(fileID);
				// sequence type, load up the movie
				if(movie != NULL){
					delete movie;
				}
				movie = new goVideoPlayer();
				movie->loadMovie(fullFilePath);
			}
			catch (JungleException je) {
				// No file existed, we expect that when we run this function
				// we've already run validateFileExistence() 
				// so if no file exists here, then we're screwed and should throw
				throw je;
			}
		}
		else if(kvmap["type"] == "transform"){
			try {
				// get file details
				fileID = findFileIDForLister(kvmap["filename"]); // excepts on missing file
				fullFilePath = _dirLister.getPath(fileID);
			}
			catch(JungleException je){
				// transform file did not exist.
				// cant continue with check, so we need to rebuild that.
				transformFilesRequired.push_back(parsedDataIter->first);
				continue; // can't check the rest of this stuff
			}

			// if movie is null, then we have not loaded a movie to compare with, so fail
			if(movie == NULL){
				//throw JungleException("No movie loaded, can not check vector vs frames for " + kvmap["filename"]);
				LOG_WARNING("No movie loaded (movie file didn't exist?), can not check frame count transform vs movie for "
							+ parsedDataIter->first + ", assuming that video will get replaced later on,"
							+ "including valid transform data");
				continue; // can't check without a movie
																												
			}			
			
			// load vector
			transform.clear();
			loadVector(fullFilePath, &transform);
			
			//check vs movie
			if(transform.size() != movie->getTotalNumFrames()){
				// mismatch, need to regen
				LOG_WARNING("Frame count mismatch for " + kvmap["filename"] 
							+ "(transform " + ofToString((int)(transform.size())) + " vs movie " 
							+ ofToString(movie->getTotalNumFrames())+")");
				transformFilesRequired.push_back(parsedDataIter->first);
			}				
			
		}
	}
	
	if(transformFilesRequired.size() != 0){
		throw transformFilesRequired; // missing transform files, throw that vector back
	}
	
}

void SceneXMLParser::parseXML1(){

	// used for creation
	Scene *scene;
	Sequence *sequence;
	vector<CamTransform> *transform;
	goVideoPlayer *movie;
	
	// conveniece stuff
	string combinedPath;
	string defaultString = "default string value should not be seen";

	// Used for validation (dateCreated,dateModified,size) and loading
	string fullFilePath = "";
	string constructedFilename = "";
	map<string, string> fileInfo; // dateCreated->"some date", etc
	
	bool hasCurrentScene = false; // used to set scene 0 to first current scene
	
	_xml.pushTag("config"); // move into root
	
	LOG_VERBOSE("Loading scene data");
	if(!_xml.tagExists("scenes")){ // quick check
		LOG_ERROR("No scenes configuration to load!");
		throw JungleException("No scenes node in configuration");
	}
	
	
	for(int sceneNum = 0; sceneNum < _xml.getNumTags("scene"); sceneNum++){
		if(!_xml.attributeExists("scene", "name", sceneNum)){
			LOG_ERROR("Could not set scene name, no name?");
		}
		
		// make new scene
		scene = new Scene();
		
		// save name
		scene->setName(_xml.getAttribute("scene", "name", defaultString, sceneNum));
		LOG_VERBOSE("Currently loading scene: " + scene->getName());
		
		// convenience
		string sceneRootPath = ofToDataPath((boost::any_cast<string>)(_appModel->getProperty("scenesDataPath"))) + "/" + scene->getName() + "/";
		
		/*
		 push into scene so we can check attributes of sequences,
		 cant just do it regularly because we have two "which" params,
		 which scene and which sequence
		 */
		_xml.pushTag("scene", sceneNum);
		
		// find the children of the scene node (ie: sequences)		
		for(int seqNum = 0; seqNum < _xml.getNumTags("sequence"); seqNum++){
			
			// create sequence
			sequence = new Sequence();
			
			// set name
			if(!_xml.attributeExists("sequence", "name", seqNum)){
				LOG_ERROR("No sequence name for sequence: "+ofToString(seqNum));
				throw JungleException("No sequence name for sequence: "+ofToString(seqNum));
			}
			sequence->setName(_xml.getAttribute("sequence", "name", defaultString, seqNum));
			LOG_VERBOSE("Currently loading sequence: " + sequence->getName());

			
			// Load movie
			
			// check that the sequence movie file matches the attributes saved in the xml (dates/size)
			constructedFilename = scene->getName()+"_"+sequence->getName()+".mov";
			fileInfo["dateCreated"] =  _xml.getAttribute("sequence", "dateCreated", defaultString, seqNum);
			fileInfo["dateModified"] = _xml.getAttribute("sequence", "dateModified", defaultString, seqNum);
			fileInfo["size"] = _xml.getAttribute("sequence", "size", defaultString, seqNum);
			if(!compareFileinfo(constructedFilename, fileInfo)){
				LOG_WARNING("File details for " + constructedFilename +" does not match xml store");
				delete sequence;
				delete scene;
				throw JungleException("File details for " + constructedFilename +" does not match xml store");
				// NOTE:	because we throw exception here, we don't check if the key exists below
				//			when we get the full file path because we assume that if we didn't fail
				//			here then its ok there. IE: Add a check if you remove the throw.
			}

			// Load the sequence movie
			movie = new goVideoPlayer();
			fullFilePath = _dirLister.getPath(_filenameToDirListerIDMap[constructedFilename]);
			printf("%s\n", fullFilePath.c_str());
			if(!movie->loadMovie(fullFilePath)){
				LOG_ERROR("Could not load movie: "+constructedFilename);
				delete movie; // free video
				delete sequence; // cant use sequence without video so fail it
				delete scene;
				abort(); // lets just assume no video for one means whole thing is broken
			};
			
			// insert the video into the sequence
			sequence->setSequenceMovie(movie);
			sequence->prepareSequenceMovie();
			
			
			// check for interactive attribute (loop videos wait for interaction)
			string iteractivityType = _xml.getAttribute("sequence", "interactivity", defaultString, seqNum);
			if(iteractivityType == "both"){
				LOG_VERBOSE(sequence->getName()+ " has interactivity attribute");
				sequence->setIsInteractive(true);
				if(!_xml.attributeExists("sequence", "victimResult", seqNum) ||
				   !_xml.attributeExists("sequence", "attackerResult", seqNum)){
					LOG_ERROR("Sequence " + sequence->getName() + " is interactive but has no victim/attacker result names");
					delete sequence;
					throw JungleException("Sequence " + sequence->getName() + " is interactive but has no victim/attacker result names");
				}
				// set attacker/victim results
				sequence->setVictimResult(_xml.getAttribute("sequence", "victimResult", defaultString, seqNum)); 
				sequence->setAttackerResult(_xml.getAttribute("sequence", "attackerResult", defaultString, seqNum)); 
			}
			else{
				LOG_VERBOSE(sequence->getName()+ " does not have 'both' interactive flag. MUST ADD PROPERTIES TO SEQUENCE TO HANDLE 'face' and 'victim (only)'");
				sequence->setIsInteractive(false);
			}
			
			// set up next sequence name
			if(_xml.attributeExists("sequence", "nextSequence", seqNum)){
				sequence->setNextSequenceName(_xml.getAttribute("sequence", "nextSequence", defaultString, seqNum));
				LOG_VERBOSE("Setting next sequence to: " + sequence->getNextSequenceName());
			}
			else{
				LOG_ERROR("No nextSequence attribute for " + sequence->getNextSequenceName());
				throw JungleException("No nextSequence attribute for " + sequence->getNextSequenceName());
			}
			
			// find transforms for this sequence
			// Push into this sequence so we insert into the right one.
			_xml.pushTag("sequence", seqNum);
			for(int transNum = 0; transNum < _xml.getNumTags("transform"); transNum++){
				transform = new vector<CamTransform>();
				string filename = _xml.getAttribute("transform", "filename", defaultString, transNum);
				if(!loadVector<CamTransform>(sceneRootPath+filename, transform)){
					// load Vector failed will log own error
					delete transform;
					throw JungleException("Could not load transform data for" + filename);
				}
				
			
				
				// check that the sequence file matches the attributes saved in the xml (dates/size)
				constructedFilename = _xml.getAttribute("transform", "filename", defaultString, transNum);
				fileInfo["dateCreated"] =  _xml.getAttribute("transform", "dateCreated", defaultString, transNum);
				fileInfo["dateModified"] = _xml.getAttribute("transform", "dateModified", defaultString, transNum);
				fileInfo["size"] = _xml.getAttribute("transform", "size", defaultString, transNum);
				if(!compareFileinfo(constructedFilename, fileInfo)){
					LOG_WARNING("File details for " + constructedFilename +" does not match xml store");
					throw JungleException("File details for " + constructedFilename +" does not match xml store");
				}

				// insert transform into sequence vector				
				sequence->addTransform(transform);				
			}
			_xml.popTag(); // pop out of sequence
			
			// made the sequence, insert it into scene
			scene->setSequence(sequence->getName(), sequence);
			if(seqNum == 0){
				// set first sequence to current sequence
				scene->setCurrentSequence(sequence->getName());
			}
			
		} // end sequence for loop
		
		_xml.popTag(); // pop out of scene
		
		// finished creation, insert
		_appModel->setScene(scene->getName(), scene);
		if(sceneNum == 0){
			// set first scene to current scene
			_appModel->setCurrentScene(scene->getName());
		}
		
	} // end scene for loop
	
	_xml.popTag(); // pop out of scenes
	
	
}

int SceneXMLParser::findFileIDForLister(string filename){
	if(_filenameToDirListerIDMap.find(filename) == _filenameToDirListerIDMap.end()){
	   LOG_ERROR("No lister id for filename " + filename);
	   throw JungleException("File not found in dirlist: " + filename);
	}
	return _filenameToDirListerIDMap.find(filename)->second;
}
		   

// Set up file lister
// resets state so it can be called whenever you want to start fresh
void SceneXMLParser::setupDirLister(){
	_dirLister.reset();
	_dirLister.addDir(ofToDataPath(_dataPath, true));
	_dirLister.allowExt("mov");
	_dirLister.allowExt("MOV");
	_dirLister.allowExt("bin");
	_dirLister.allowExt("BIN");
	_numFiles = _dirLister.listDir(true);
	// quick error check
	if(_numFiles == 0){
		LOG_ERROR("Could not build scene config, goDirList reported 0 files found.");
		abort();
	}
	
}

void SceneXMLParser::populateDirListerIDMap(){
	// iterate over all files, put its name and the id for that file in the map
	for(int fileNum = 0; fileNum < _numFiles; fileNum++){
		_filenameToDirListerIDMap.insert(make_pair(_dirLister.getName(fileNum), fileNum));
	}
}


