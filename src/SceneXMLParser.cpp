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
	
	setupLister(); // set up file lists;
	populateListerIDMap();
	load(); // load xml
	parseXML(); // make model from xml (and validate against size/date
}

void SceneXMLParser::parseXML(){

	// used for creation
	Scene *scene;
	Sequence *sequence;
	vector<CamTransform> *transform;
	goVideoPlayer *video;
	
	// conveniece stuff
	string combinedPath;
	string defaultString = "default string value should not be seen";

	// Used for validation (dateCreated,dateModified,size)
	string calculatedFilename = "";
	map<string, string> fileInfo; // dateCreated->"some date", etc
	
	bool hasCurrentScene = false; // used to set scene 0 to first current scene
	
	_xml.pushTag("config"); // move into root
	
	LOG_VERBOSE("Loading scene data");
	if(!_xml.tagExists("scenes")){ // quick check
		LOG_ERROR("No scenes configuration to load!");
		throw JungleException("No scenes node in configuration");
	}
	
	// set scenes to root
	_xml.pushTag("scenes");
	
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
			
			// check that the sequence file matches the attributes saved in the xml (dates/size)
			calculatedFilename = scene->getName()+"_"+sequence->getName()+".mov";
			fileInfo["dateCreated"] =  _xml.getAttribute("sequence", "dateCreated", defaultString, seqNum);
			fileInfo["dateModified"] = _xml.getAttribute("sequence", "dateModified", defaultString, seqNum);
			fileInfo["size"] = _xml.getAttribute("sequence", "size", defaultString, seqNum);
			if(!compareFileinfo(calculatedFilename, fileInfo)){
				LOG_WARNING("File details for " + calculatedFilename +" does not match xml store");
				throw JungleException("File details for " + calculatedFilename +" does not match xml store");
			}
		
			
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
				if(!sequence->getIsInteractive()){
					// seq isnt interactive, so no nextSequence attribute means its the final sequence
					sequence->setNextSequenceName(kLAST_SEQUENCE_TOKEN);
					LOG_VERBOSE("Setting next sequence to: " + kLAST_SEQUENCE_TOKEN);
				}
				else{
					/*	just to make sure its set to something, techinically, a "no
					 next sequence" sequence loops, so it is its own next sequence */				
					sequence->setNextSequenceName(sequence->getName());
				}
			}
			
			// find transforms for this sequence
			// again, push into this sequence
			_xml.pushTag("sequence", seqNum);
			for(int transNum = 0; transNum < _xml.getNumTags("transform"); transNum++){
				transform = new vector<CamTransform>();
				if(!loadVector<CamTransform>(sceneRootPath+_xml.getAttribute("transform", "filename", defaultString, transNum),
											 transform)){
					// load Vector failed will log own error
					delete transform;
					throw JungleException("Could not load transform data");
				}
				// insert transform into sequence vector
				// check that the sequence file matches the attributes saved in the xml (dates/size)
				calculatedFilename = _xml.getAttribute("transform", "filename", defaultString, transNum);
				fileInfo["dateCreated"] =  _xml.getAttribute("transform", "dateCreated", defaultString, transNum);
				fileInfo["dateModified"] = _xml.getAttribute("transform", "dateModified", defaultString, transNum);
				fileInfo["size"] = _xml.getAttribute("transform", "size", defaultString, transNum);
				if(!compareFileinfo(calculatedFilename, fileInfo)){
					LOG_WARNING("File details for " + calculatedFilename +" does not match xml store");
					throw JungleException("File details for " + calculatedFilename +" does not match xml store");
				}
				
				sequence->addTransform(transform);				
			}
			_xml.popTag(); // pop out of sequence
			
			// Load the sequence movie
			video = new goVideoPlayer();
			combinedPath = sceneRootPath+scene->getName() + "_" + sequence->getName()+".mov";
			if(!video->loadMovie(combinedPath)){
				LOG_ERROR("Could not load movie: "+combinedPath);
				delete video; // free video
				delete sequence; // cant use sequence without video so fail it
				abort(); // lets just assume no video for one means whole thing is broken
			};
			// TODO: video->play(); vidoe->setPause(true);
			
			sequence->setSequenceMovie(video);
			sequence->prepareSequenceMovie();
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

bool SceneXMLParser::compareFileinfo(string filename, map<string, string> fileInfo){
	if(_filenameToListerIDMap.find(filename) == _filenameToListerIDMap.end()){
		LOG_ERROR("No lister id for filename " + filename + " can not check details");
		abort();
	}
	int listerID = _filenameToListerIDMap.find(filename)->second;

	//LOG_NOTICE("Incoming: lister vs fileinfo - \n\t" + _lister.getCreated(listerID) + " : " + fileInfo["dateCreated"] + ",\n\t" + _lister.getModified(listerID) + " : " + fileInfo["dateModified"] + ",\n\t" + ofToString(_lister.getSize(listerID)) + " : " + fileInfo["size"]);
	
	bool retv = true;
	if(_lister.getCreated(listerID) != fileInfo["dateCreated"]){
		LOG_WARNING("dateCreated mismatch on " + filename + "("+_lister.getCreated(listerID) + "hdd vs " + fileInfo["dateCreated"]+"xml)");
		retv = false;
	}
	if(_lister.getModified(listerID) != fileInfo["dateModified"]){
		LOG_WARNING("dateModified mismatch on " + filename + "("+_lister.getCreated(listerID) + "hdd vs " + fileInfo["dateModified"]+"xml)");
		retv = false;
	}
	if(ofToString(_lister.getSize(listerID)) != fileInfo["size"]){
		LOG_WARNING("size mismatch on " + filename + "("+ofToString(_lister.getSize(listerID)) + "hdd vs " + fileInfo["size"]+"xml)");
		retv = false;
	}
	return retv;
}

// Set up file lister
// resets state so it can be called whenever you want to start fresh
void SceneXMLParser::setupLister(){
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

void SceneXMLParser::populateListerIDMap(){
	// iterate over all files, put its name and the id for that file in the map
	for(int fileNum = 0; fileNum < _numFiles; fileNum++){
		_filenameToListerIDMap.insert(make_pair(_lister.getName(fileNum), fileNum));
	}
}


