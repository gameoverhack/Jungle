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
	LOG_VERBOSE("Initialising with datapath: " + dataPath + " and config: " + xmlFile);
	_dataPath = dataPath;
	_movie = new goVideoPlayer();
	LOG_VERBOSE("SceneXMLParser Initialised");
}

void SceneXMLParser::registerStates() {
	LOG_VERBOSE("Registering States");
	
	registerState(kSCENEXMLPARSER_INIT, "kSCENEXMLPARSER_INIT");
	registerState(kSCENEXMLPARSER_SETUP, "kSCENEXMLPARSER_SETUP");
	registerState(kSCENEXMLPARSER_PARSE_XML, "kSCENEXMLPARSER_PARSE_XML");
	registerState(kSCENEXMLPARSER_VALIDATING_FILE_METADATA, "kSCENEXMLPARSER_VALIDATING_FILE_METADATA");
	registerState(kSCENEXMLPARSER_VALIDATING_MOVIE_FILE_EXISTENCE, "kSCENEXMLPARSER_VALIDATING_MOVIE_FILE_EXISTENCE");
	registerState(kSCENEXMLPARSER_VALIDATING_MOVIE_TRANSFORM_LENGTHS, "kSCENEXMLPARSER_VALIDATING_MOVIE_TRANSFORM_LENGTHS");
	registerState(kSCENEXMLPARSER_CREATING_APPMODEL, "kSCENEXMLPARSER_CREATING_APPMODEL");
	registerState(kSCENEXMLPARSER_FINISHED, "kSCENEXMLPARSER_FINISHED");
	
	setState(kSCENEXMLPARSER_INIT);
}

void SceneXMLParser::update() {
	
	switch (getState()) {
			
		case kSCENEXMLPARSER_INIT:
			// nothing to do for init, move on to next state.
			setState(kSCENEXMLPARSER_SETUP);
			break;
			
		case kSCENEXMLPARSER_SETUP:
			// setup whats needed to parse
			setupDirLister(); // set up file lists;
			populateDirListerIDMap();
			load(); // load xml file
			setState(kSCENEXMLPARSER_PARSE_XML);
			break;
			
		case kSCENEXMLPARSER_PARSE_XML:
			// make map (map<string, map<string, string> _parsedData) from xml
			parseXML();
			listParsedData();
			setState(kSCENEXMLPARSER_VALIDATING_MOVIE_FILE_EXISTENCE);
			break;
		case kSCENEXMLPARSER_VALIDATING_MOVIE_FILE_EXISTENCE:
			// Check that the map contains valid files (replace with temp fakes if it doesnt)
			// throws JungleException if some files can't be fixed that should cause a crash since we can't go on at all.
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
			setState(kSCENEXMLPARSER_VALIDATING_FILE_METADATA);
			break;
			
		case kSCENEXMLPARSER_VALIDATING_FILE_METADATA:
			try{
				validateFileMetadata();
			}
			catch(vector<string> vec){
				// Some files were invalid, we should report this, and throw an exception
				// who ever caled us should created a SceneXMLBuilder and rebuild the xml
				string message = "";
				for(vector<string>::iterator iter = vec.begin(); iter != vec.end(); iter++){
					message = *iter + ", " + message;
				}
				message[message.length()-1] = ' ';
				message = "Metadata xml vs hdd mismatch for files: " + message;
				LOG_ERROR(message);
				throw MetadataMismatchException(message);
			}
			setState(kSCENEXMLPARSER_VALIDATING_MOVIE_TRANSFORM_LENGTHS);
			break;
			
		case kSCENEXMLPARSER_VALIDATING_MOVIE_TRANSFORM_LENGTHS:
			// Validate that transform and movie lengths are the same
			if(validateMovieTransformLengths()){
				// reset completedKeys set
				_completedKeys.clear();

				// done, goto next state
				setState(kSCENEXMLPARSER_CREATING_APPMODEL);

				// Check if we saved any broken pairs
				if(_missingTransforms.size() != 0){
					// missing transform files, throw exception
					throw TransformMovieLengthMismatchException("Transform and movie lengths mismatch", _missingTransforms);
				}
			}
			break;
			
		case kSCENEXMLPARSER_CREATING_APPMODEL:
		{
			// Checked file existence, checked metadata, checked transforms, OK to map => app model
			//			listParsedData();
			
			if(createAppModel()){
				_completedKeys.clear();
				setState(kSCENEXMLPARSER_FINISHED);
			};
			break;
		}
		case kSCENEXMLPARSER_FINISHED:
			break;
			
		default:
			LOG_ERROR("Unknown state " + printState());
			break;
	}
	
	// update internal loading state (datacontroller will use this to set app messages)
	updateLoadingState();
}

// Convenience function
void SceneXMLParser::updateLoadingState(){
	// loading messave is just the apps state.
	switch (getState()) {
		case kSCENEXMLPARSER_INIT:
			_loadingProgress = 0.1f;
			break;
		case kSCENEXMLPARSER_SETUP:
			_loadingProgress = 0.2f;
			//_stateMessage = "Setting up dirList, populating file list map, loading XML file into memory...";
			break;
		case kSCENEXMLPARSER_PARSE_XML:
			_loadingProgress = 0.3f;
			//_stateMessage = "Parsing XML...";
			break;
		case kSCENEXMLPARSER_VALIDATING_MOVIE_FILE_EXISTENCE:
			_loadingProgress = 0.4f;
			//_stateMessage = "Validating movie file existance...";
			break;
		case kSCENEXMLPARSER_VALIDATING_FILE_METADATA:
			_loadingProgress = 0.5f;
			//_stateMessage = "Validating file metadata (size, dateCreated, dateModified)...";
			break;
		case kSCENEXMLPARSER_VALIDATING_MOVIE_TRANSFORM_LENGTHS:
			_loadingProgress = 0.6f;
			// state message set inside validateMovieTransformLenths method
			//			_stateMessage = "Validating movie and transform frame counts...";
			break;
		case kSCENEXMLPARSER_CREATING_APPMODEL:
			_loadingProgress = 0.7f;
			//			_stateMessage = "Creating app model from validated data...";
			break;
		case kSCENEXMLPARSER_FINISHED:
			//_stateMessage = "Parser finished.";
			_loadingProgress = 1.0f;
			break;
		default:
			break;
	}
}


// Convert from map to model.
// Should be all set up correctly like so:
//
// scene => map of scene data
// scene/sequence => map of sequence data
// sence/sequence/transform => map of transform data
// scene2 ...
// scene/sequence ...
// ... etc.
//
// Even if some files are missing, SceneXMLBuilder will add nodes for scenes/sequences
// if they are ever referenced by other files.
// The map is sorted alphabetically, so scenes are not in the order they are in the
// xml, but there is a scene["orderInXML"] => int (eg: 0, 1, 2) which could be
// checked when deciding what to use for the first scene.
bool SceneXMLParser::createAppModel(){
	// used for object construction
	Scene *scene = NULL;
	Sequence *sequence = NULL;
	vector<CamTransform> *transform = NULL;
	goVideoPlayer *movie;
	
	// file retrieval
	int fileID;
	string fullFilePath;
	
	bool hasCurrentScene = false; // used to set scene 0 to first current scene
	bool hasCurrentSequence = false;
	
	// check if total number of parsed keys matches total number of completed keys
	if(_parsedData.size() == _completedKeys.size()){
		return true; //we've checked all the keys
	}
	
	// Initial iteration, find all scenes, add them to app model
	// We have to do this separately beause we want per-sequence
	// update message granularity
	map<string, map<string, string> >::iterator parsedDataIter;
	for(parsedDataIter = _parsedData.begin(); parsedDataIter != _parsedData.end(); parsedDataIter++){
		
		if(_completedKeys.find(parsedDataIter->first) != _completedKeys.end()){
			continue; // already done this key
		}
		
		map<string, string> & kvmap = (parsedDataIter->second); // syntax convenience
		if(kvmap["type"] == "scene"){
			LOG_VERBOSE("xml => model for " + kvmap["name"] + " ("+kvmap["type"]+")");
			scene = new Scene();
			scene->setName(kvmap["name"]);
			_appModel->setScene(scene->getName(), scene);
			if(hasCurrentScene == false){
				_appModel->setCurrentScene(scene->getName());
				hasCurrentScene = true;
			}
			_completedKeys.insert(parsedDataIter->first); // key done, save it
		}
	}
	
	// loop over all the keys, find which are sequences (scene:sequence, so two parts)
	for(parsedDataIter = _parsedData.begin(); parsedDataIter != _parsedData.end(); parsedDataIter++){
		
		if(_completedKeys.find(parsedDataIter->first) != _completedKeys.end()){
			continue; // already done this key
		}
		//_stateMessage = "Creating app model for: " + parsedDataIter->first;
		
		// split
		vector<string> keyParts;
		boost::split(keyParts, parsedDataIter->first, boost::is_any_of(":"));
		
		if(keyParts.size() != 2){
			continue; // not a sequence
		}
		
		// is a sequence, so get the scene for this sequence
		scene = _appModel->getScene(keyParts[0]);
		
		// Set up this sequence (including the transforms)
		map<string, string> & kvmap = (parsedDataIter->second); // syntax convenience
		
		// should be a sequence but check anyway
		if(kvmap["type"] != "sequence"){
			throw JungleException("Thought " + parsedDataIter->first + " was  sequence but its type was " + kvmap["type"]);
		}
		
		LOG_VERBOSE("xml => model for " + kvmap["name"] + " ("+kvmap["type"]+")");
		
		// actually build the sequence parts
		sequence = new Sequence();
		sequence->setName(kvmap["name"]);
		sequence->setNextSequenceName(kvmap["nextSequence"]);
		
		// save some meta data about the sequence
		sequence->setType(kvmap["sequenceType"]);
		sequence->setNumber(findSequenceNumberFromString(sequence->getName()));
		
		// loop only stuff
		if(kvmap["sequenceType"] == "loop"){
			sequence->setAttackerResult(kvmap["attackerResult"]);
			sequence->setVictimResult(kvmap["victimResult"]);
		} else if(kvmap["sequenceType"] == "a"){
			sequence->setVictimResult(kvmap["victimResult"]);
		}
		
		// set if sequence is faked
		if(kvmap["faked"] == "true"){
			sequence->setIsSequenceFaked(true);
		}
		
		// set interactivity
		sequence->setInteractivity(kvmap["interactivity"]);
		
		// set whether we faked movie data
		if(kvmap["validFile"] == "fake"){
			sequence->setIsMovieFaked(true);
		}
		
		// Load movie stuff
		fileID = findFileIDForLister(kvmap["filename"]);
		fullFilePath = _dirLister.getPath(fileID);
		sequence->setMovieFullFilePath(fullFilePath);
		
		// load interactivity stuff
		
		// set up the descriptor
		SequenceDescriptor *descriptor = new SequenceDescriptor();
		// find and laod the file
		loadClass(findFullFilePathForFilename(kvmap["interactivityFilename"]), descriptor);
		vector<FramePair>::iterator vecIter;

		interaction_t *interactionTable;
		
		// get total number of frames, set up array 
		interactionTable = new interaction_t[descriptor->_totalFrames];
		
		// "zero" out the table with kINTERACTION_NONE
		for(int i = 0; i < descriptor->_totalFrames; i++){
			interactionTable[i] = kINTERACTION_NONE;
		}
		
		// set up face
		for(vecIter = descriptor->_face.begin(); vecIter != descriptor->_face.end(); vecIter++){
			FramePair fp = *vecIter;
			for(int i = fp._start; i < fp._end; i++){
					interactionTable[i] = kINTERACTION_FACE;
			}
		}
		
		// set up victim
		for(vecIter = descriptor->_victim.begin(); vecIter != descriptor->_victim.end(); vecIter++){
			FramePair fp = *vecIter;
			for(int i = fp._start; i < fp._end; i++){
				interactionTable[i] = kINTERACTION_VICTIM;
			}
		}
		
		// set up attacker
		for(vecIter = descriptor->_attacker.begin(); vecIter != descriptor->_attacker.end(); vecIter++){
			FramePair fp = *vecIter;
			for(int i = fp._start; i < fp._end; i++){
				if(interactionTable[i] == kINTERACTION_VICTIM){
					interactionTable[i] = kINTERACTION_BOTH;
				}
				else{
					interactionTable[i] = kINTERACTION_ATTACKER;
				}				
			}
		}
		
		
		// save table
		sequence->setInteractionTable(interactionTable);
													
		// print out result table
		
for(int i = 0; i < descriptor->_totalFrames; i++){
	string s = "";
	if (interactionTable[i] == kINTERACTION_NONE) {
		s = "NONE";
	}
	if (interactionTable[i] == kINTERACTION_VICTIM) {
		s = "VICTIM";
	}
	if (interactionTable[i] == kINTERACTION_ATTACKER) {
		s = "ATTACKER";
	}
	if (interactionTable[i] == kINTERACTION_FACE) {
		s = "FACE";
	}

	if (interactionTable[i] == kINTERACTION_BOTH) {
		s = "BOTH";
	}
	
	printf("%3d: %s\n", i, s.c_str());
}
		
		
		// completed sequence, insert to scene
		scene->setSequence(sequence->getName(), sequence);
		
		// check if we're first sequence for this scene, if we are, set us to be current (ie first)
		if(!scene->hasCurrentSequence()){
			scene->setCurrentSequence(sequence->getName());
			
		}
		
		// Find the transforms for this sequence
		regex transformPattern("^"+parsedDataIter->first + ":[^:]+?$");
		map<string, map<string, string> >::iterator transformIter;
		for(transformIter = _parsedData.begin(); transformIter != _parsedData.end(); transformIter++){
			if(!regex_search(transformIter->first, transformPattern)){
				continue; // NOT matched, keep looking
			}
			
			kvmap = transformIter->second; // convenience
			
			// should be a transform but check
			if(kvmap["type"] != "transform"){
				throw JungleException("Thought " + transformIter->first + " was  transform but its type was " + kvmap["type"]);
			}
			
			LOG_VERBOSE("xml => model for " + kvmap["name"] + " ("+kvmap["type"]+")");
			
			// do the transform stuff
			transform = new vector<CamTransform>();
			loadVector(findFullFilePathForFilename(kvmap["filename"]), transform);
			vector<string> keyParts;
			boost::split(keyParts, transformIter->first, boost::is_any_of(":"));
			sequence->setTransform(keyParts[2], transform);
			_completedKeys.insert(transformIter->first); // key done, save it
		}

		_completedKeys.insert(parsedDataIter->first); // key done, save it
		break; // break out of the loop
	}
	return false;
}



// Parse xml into a map structure which we can then build the model from in populateAppModel
void SceneXMLParser::parseXML(){
	string sceneName, sequenceName, transformName, mapKey;
	string stringType = "default string, if you see this, getAttribute failed";
	vector<string> attributesToCheck;

	// Set up _xml root node
	if(!_xml.tagExists("config")){ // quick check
		LOG_ERROR("No config node in xml");
		throw GenericXMLParseException("No config and scenes node in xml");
	}
	_xml.pushTag("config");
	LOG_VERBOSE("Parsing scene data");
	if(!_xml.tagExists("scenes")){ // quick check
		LOG_ERROR("No scenes node in xml!");
		throw GenericXMLParseException("No scenes node in xml");
	}
	_xml.pushTag("scenes");
	// document root acceptable
	
	// sanity check
	if(_xml.getNumTags("scene") == 0){
		LOG_ERROR("No scene nodes in xml");
		throw GenericXMLParseException("No scene nodes to xml");
	}
	
	// push into each scene node
	for(int sceneNum = 0; sceneNum < _xml.getNumTags("scene"); sceneNum++){
		
		// check attributes
		attributesToCheck.clear();
		attributesToCheck.push_back("name");
		checkTagAttributesExist("scene", attributesToCheck, sceneNum);
		
		// Remember the name
		sceneName = _xml.getAttribute("scene", "name", stringType, sceneNum);
		LOG_VERBOSE("Parsing xml=>map scene: " + sceneName);
		
		mapKey = sceneName;
		_parsedData[mapKey]["name"] = sceneName;
		_parsedData[mapKey]["type"] = "scene";
		_parsedData[mapKey]["positionInXML"] = ofToString(sceneNum);
		
		// set new root
		_xml.pushTag("scene", sceneNum);
		
		// sanity check
		if(_xml.getNumTags("sequence") == 0){
			LOG_ERROR("No sequence nodes for scene in xml");
			throw GenericXMLParseException("No sequence nodes for scene in xml");
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
			attributesToCheck.push_back("nextSequence");
			attributesToCheck.push_back("faked");
			attributesToCheck.push_back("sequenceType");
			attributesToCheck.push_back("filename");
			checkTagAttributesExist("sequence", attributesToCheck, seqNum);
			
			// Remember the sequence name
			sequenceName = _xml.getAttribute("sequence", "name", stringType, seqNum);
			LOG_VERBOSE("Parsing xml=>map sequence: " + sequenceName);
			
			// check for loop only attributes
			if(_xml.getAttribute("sequence", "sequenceType", stringType, seqNum) == "loop"){
				attributesToCheck.push_back("attackerResult");
				attributesToCheck.push_back("victimResult");
				checkTagAttributesExist("sequence", attributesToCheck, seqNum);
			}
			
			// done checking, sequence node has all we'll need so build it up
			
			// construct key for parsed data map, rebuild instead of just append to be clearer
			mapKey = sceneName+":"+sequenceName;
			
			// save some details
			_parsedData[mapKey]["name"] = sequenceName;
			_parsedData[mapKey]["type"] = "sequence";
			_parsedData[mapKey]["faked"] = _xml.getAttribute("sequence", "faked", stringType, seqNum);
			_parsedData[mapKey]["sequenceType"] = _xml.getAttribute("sequence", "sequenceType", stringType, seqNum);
			_parsedData[mapKey]["nextSequence"] = _xml.getAttribute("sequence", "nextSequence", stringType, seqNum);
			
			if(_parsedData[mapKey]["sequenceType"] == "loop"){
				_parsedData[mapKey]["attackerResult"] = _xml.getAttribute("sequence", "attackerResult", stringType, seqNum);
				_parsedData[mapKey]["victimResult"] = _xml.getAttribute("sequence", "victimResult", stringType, seqNum);
			} else{
				// check if its an a or b
				if(_parsedData[mapKey]["sequenceType"] == "a"){
					// a type sequences have a victim result
					attributesToCheck.push_back("victimResult");
					checkTagAttributesExist("sequence", attributesToCheck, seqNum);
					_parsedData[mapKey]["victimResult"] = _xml.getAttribute("sequence", "victimResult", stringType, seqNum);
				} else {
					// b type, nothing to set.
				}
			}
			
			// save movie file name
			_parsedData[mapKey]["filename"] = _xml.getAttribute("sequence", "filename", stringType, seqNum);
			_parsedData[mapKey]["validFile"] = "unvalidated"; // haven't checked file yet
						
			// save all the file meta data
			_parsedData[mapKey]["size"] = _xml.getAttribute("sequence", "size", stringType, seqNum);
			_parsedData[mapKey]["dateCreated"] = _xml.getAttribute("sequence", "dateCreated", stringType, seqNum);
			_parsedData[mapKey]["dateModified"] = _xml.getAttribute("sequence", "dateModified", stringType, seqNum);
			
			// save interactivity info
			_parsedData[mapKey]["interactivity"] = _xml.getAttribute("sequence", "interactivity", stringType, seqNum);
			
			// find transforms for this sequence
			// Push into this sequence so we insert into the right one.
			_xml.pushTag("sequence", seqNum);
			
			if(_xml.getNumTags("interactivity") == 0){
				LOG_ERROR("No interactivity node for sequence " + mapKey + " in xml");
				// check if it was faked
				if(_parsedData[mapKey]["faked"] == "true"){					
					LOG_VERBOSE(mapKey + " was fake, will not try to fix");
				}
				else{
					_missingTransforms.push_back(mapKey); // save map key to rebuild
				}
				
			}
			else{
				// check interactivity
				attributesToCheck.clear();
				attributesToCheck.push_back("filename");
				checkTagAttributesExist("interactivity", attributesToCheck, 0);
				
				// create interactivity filename
				_parsedData[mapKey]["interactivityFilename"] = _xml.getAttribute("interactivity", "filename", stringType, 0);
			}
			
			// sanity check
			if(_xml.getNumTags("transform") == 0){
				LOG_ERROR("No transform nodes for sequence " + mapKey + " in xml");
				// check if it was faked
				if(_parsedData[mapKey]["faked"] == "true"){					
					LOG_VERBOSE(mapKey + " was fake, will not try to fix");
				}
				else{
					_missingTransforms.push_back(mapKey); // save map key to rebuild
				}
				
			}
			for(int transNum = 0; transNum < _xml.getNumTags("transform"); transNum++){
				// check attributes
				attributesToCheck.clear();
				attributesToCheck.push_back("filename");
				attributesToCheck.push_back("size");
				attributesToCheck.push_back("dateModified");
				attributesToCheck.push_back("dateCreated");
				attributesToCheck.push_back("faked");
				checkTagAttributesExist("transform", attributesToCheck, transNum);
				
				// get atk1/2/vic1 from filename.
				transformName = _xml.getAttribute("transform", "filename", stringType, transNum);
				vector<string> splitName;
				boost::split(splitName, transformName, boost::is_any_of("_"));
				transformName = splitName[splitName.size() -1]; // just use atk1/vic etc
				transformName = transformName.substr(0, transformName.find_last_of(".")); // remove ext
				
				// set up map key
				mapKey = sceneName+":"+sequenceName+":"+transformName;
				
				_parsedData[mapKey]["name"] = transformName;
				_parsedData[mapKey]["type"] = "transform";
				_parsedData[mapKey]["faked"] = _xml.getAttribute("transform", "faked", stringType, transNum);
				_parsedData[mapKey]["filename"] = _xml.getAttribute("transform", "filename", stringType, transNum);
				_parsedData[mapKey]["size"] = _xml.getAttribute("transform", "size", stringType, transNum);
				_parsedData[mapKey]["dateCreated"] = _xml.getAttribute("transform", "dateCreated", stringType, transNum);
				_parsedData[mapKey]["dateModified"] = _xml.getAttribute("transform", "dateModified", stringType, transNum);
			}
			
			_xml.popTag(); // pop sequence
			LOG_VERBOSE("Finished xml=>map sequence: " + sequenceName);
		}
		_xml.popTag(); // pop scene
		LOG_VERBOSE("Finishsed xml=>map scene: " + sceneName);
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
			if(regex_search(invalidKeySplit[1], regex("a$"))){
				regexPattern = regexPattern + "a$";
			}
			else{
				regexPattern = regexPattern + "b$";
			}
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
				LOG_WARNING("Copying movie from " + good + " to " + bad);
				
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
		LOG_VERBOSE("Validating file metadata for " + parsedDataIter->first);
		
		if(kvmap["type"] == "scene"){
			continue;
		}
		// check that the map has the right keys
		if(kvmap.find("size") == kvmap.end() ||
		   kvmap.find("dateModified") == kvmap.end() ||
		   kvmap.find("dateCreated") == kvmap.end() ||
		   kvmap.find("filename") == kvmap.end() ){
			throw JungleException("Required one of kvmap keys filename, size, dateModified \
								  or dateCreated missing for " + parsedDataIter->first);
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
bool SceneXMLParser::validateMovieTransformLengths(){
	goVideoPlayer *movie = NULL;
	vector<CamTransform> transform;
	
	int fileID;
	string fullFilePath;
	
	vector<string> transformFilesRequired;
	
	// check if total number of parsed keys matches total number of completed keys
	if(_parsedData.size() == _completedKeys.size()){
		return true; //we've checked all the keys
	}
	
	// iterate over the whole parsed data map
	map<string, map<string, string> >::iterator parsedDataIter;
	for(parsedDataIter = _parsedData.begin(); parsedDataIter != _parsedData.end(); parsedDataIter++){
		// Check if we've already processed
		if(_completedKeys.find(parsedDataIter->first) != _completedKeys.end()){
			continue; // already done this key
		}
		//_stateMessage = "Validating movie and transform lengths: " + parsedDataIter->first;
		
		map<string, string> & kvmap = (parsedDataIter->second); // syntax convenience
		LOG_VERBOSE("Validating movie+transform length for " + parsedDataIter->first);
		
		// make sure filename is exists
		if(kvmap["type"] == "sequence" || kvmap["type"] == "transform"){
			if(kvmap.find("filename") == kvmap.end()){
				LOG_ERROR("No filename attribute in kvmap for " + kvmap["name"]);
				throw JungleException("No filename attribute in kvmap for " + parsedDataIter->first);
			}
		}
		
		if (kvmap["type"] == "scene") {
			// bit of a hack, so we can do the parsedData.size == completedKeys.size, we have to
			// add in the scenes too. We could be smarter are iterate the whole map, find out if they are
			// sequenes or transforms, then check them vs the completedKeys, and then set a final state
			// variable as "finished" if they're all present.
			_completedKeys.insert(parsedDataIter->first);
		}
		
		// Only care about sequences
		if(kvmap["type"] == "sequence"){
//			if(kvmap["faked"] == "true"){
//				// don't attempt to check fake sequences
//				LOG_VERBOSE("Not checking transforms for " + parsedDataIter->first + " because it is fake");
//				_completedKeys.insert(parsedDataIter->first);
//				break; //break out of for (yes this is what we want to do)
//			}
	
			try {
				fileID = findFileIDForLister(kvmap["filename"]);
				fullFilePath = _dirLister.getPath(fileID);
				// sequence type, load up the movie
				//if(_movie != NULL){
				//	delete _movie;
				//}
				int timea = ofGetElapsedTimeMillis();
				//_movie = new goVideoPlayer();
				_movie->setUseTexture(false);
				_movie->loadMovie(fullFilePath);
				//printf("check movie load time: %d\n", ofGetElapsedTimeMillis() - timea);
				
			}
			catch (JungleException je) {
				// No file existed, we expect that when we run this function
				// we've already run validateFileExistence()
				// so if no file exists here, then we're screwed and should throw
				throw je;
			}
			
			// find all the keys which are like this sequence key
			// scene:sequence => scene:sequence:atk1 etc
			regex similarPattern(parsedDataIter->first + ":.+?$");
			map<string, map<string, string> >::iterator innerIter;
			for(innerIter = _parsedData.begin(); innerIter != _parsedData.end(); innerIter++){
				if(!regex_search(innerIter->first, similarPattern)){
					continue; // NOT matched, keep looking
				}
				
				// DID match, so check stuff
				map<string, string> & maptransform = (innerIter->second); // syntax convenience
				if(maptransform["faked"] == "true"){
					// don't attempt to check fake sequences
					LOG_VERBOSE("Not checking transforms for " + parsedDataIter->first + " because it is fake");
					_completedKeys.insert(innerIter->first);
					continue; //continue out of for (yes this is what we want to do)
				}
				
				
				try {
					// get file details
					fileID = findFileIDForLister(maptransform["filename"]); // excepts on missing file
					fullFilePath = _dirLister.getPath(fileID);
				}
				catch(JungleException je){
					// transform file did not exist.
					// cant continue with check, so we need to rebuild that.
					_missingTransforms.push_back(innerIter->first);
					continue; // can't check the rest of this stuff
				}
				
				// load vector
				transform.clear();
				loadVector(fullFilePath, &transform);
				//check vs movie
				if(transform.size() != _movie->getTotalNumFrames()){
					// mismatch, need to regen
					LOG_WARNING("Frame count mismatch for " + innerIter->first
								+ "(transform " + ofToString((int)(transform.size())) + " vs "+ kvmap["filename"] +" movie "
								+ ofToString(_movie->getTotalNumFrames())+")");
					// Store a list of broken pairs
					_missingTransforms.push_back(innerIter->first);
				}
				// save key as processesd
				_completedKeys.insert(innerIter->first);
			}
			
			// clean up the movie
			_movie->close();
			//delete movie;
			
			// save the key as processed
			_completedKeys.insert(parsedDataIter->first);
			break;	// break out of the iteration because we've done one check
			// we should get recalled, which will cause us to check
			// against the completed keys set, and we'll skip any we've done already
		}
	}
	
	return false; // still got keys to check
}


// Set up file lister
// resets state so it can be called whenever you want to start fresh
void SceneXMLParser::setupDirLister(){
	_dirLister.reset();
	_dirLister.addDir(_dataPath);
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

// iterate over all files, put its name and the id for that file in the map
// name => id
// This is so we can find file names from the XML, and match them to a goDirLister
// id, and then use that ID to find file meta data.
void SceneXMLParser::populateDirListerIDMap(){
	for(int fileNum = 0; fileNum < _numFiles; fileNum++){
		_filenameToDirListerIDMap.insert(make_pair(_dirLister.getName(fileNum), fileNum));
	}
}

// Used to loop up a filename from the lister map
// Wrapped in a function instead of just doing _lister[filename]
// so we can throw exceptions for it consistently
int SceneXMLParser::findFileIDForLister(string filename){
	if(_filenameToDirListerIDMap.find(filename) == _filenameToDirListerIDMap.end()){
		LOG_ERROR("No lister id for filename '" + filename +"'");
		throw JungleException("File not found in dirlist: " + filename);
	}
	return _filenameToDirListerIDMap.find(filename)->second;
}

// convenience.
string SceneXMLParser::findFullFilePathForFilename(string filename){
	int fileID = findFileIDForLister(filename);
	return _dirLister.getPath(fileID);
}


// Expects to be pushed/popped into the correct level
// checks that the given attribute exist for the given tag.
// Throws GenericXMLParseException with a message containing which attributes were missing
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
		message = "Tag '" + xmltag + "'("+ofToString(which)+") missing attributes: " + message;
		// throw exception
		LOG_ERROR(message);
		throw GenericXMLParseException(message);
	}
}

float SceneXMLParser::getLoadingProgress(){
	return _loadingProgress;
}

void SceneXMLParser::listParsedData(){
	map<string, map<string, string> >::iterator iter;
	iter = _parsedData.begin();
	while (iter != _parsedData.end()) {
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

int SceneXMLParser::findSequenceNumberFromString(string str){
	int seqNum;
	char seqNumString[20];
	string matchstring;
	cmatch match;
	
	// pull out the sequence number from the sequence name, so we can increment it
	if(regex_match(str.c_str(), match, regex("seq(\\d+)\\D+?"))){ // find int
		matchstring = string(match[1].first, match[1].second);
		sscanf(matchstring.c_str(), "%d", &seqNum); // extract int
		return seqNum;
	}
	LOG_ERROR("Could not find sequence number from string: " + str);
	abort();
	
}
