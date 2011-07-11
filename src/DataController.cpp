/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"

DataController::DataController(string configFilePath){
	LOG_NOTICE("Initialising with " + configFilePath);
	
	_xml = new ofxXmlSettings();
	if(!_xml->loadFile(configFilePath)){
		LOG_ERROR("Could not load config: " + configFilePath);
		abort();
	}
	_xml->pushTag("config"); // Get inside root node
	
	
	propertyXMLParser(configFilePath);
	propertyXMLBuilder(configFilePath);
	sceneXMLBuilder("some file");
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
	delete _xml;
}


bool DataController::propertyXMLParser(string propertyConfigFilepath){
	//  load up the property xml file 
	ofxXmlSettings *xml = new ofxXmlSettings();
	if(!xml->loadFile(propertyConfigFilepath)){
		LOG_ERROR("Could not load property config: " + propertyConfigFilepath);
		abort();		
	}
	
	//  pull out the properties 
	string defaultString = "this should never be seen"; // getValue requires type to know what to return
	string propName, propValue, propType;
	
	LOG_NOTICE("Discovering properties");
	// Find all the property tags
	if(!_xml->tagExists("properties")){
		LOG_ERROR("No properties node in config");
		abort();
	}
	_xml->pushTag("properties"); // set properties as root node
	
	// loop over all property tags
	for(int i = 0; i < _xml->getNumTags("property"); i++){
		propName = _xml->getAttribute("property",
									  "name", defaultString, i);
		propType = _xml->getAttribute("property",
									  "type", defaultString, i);
		
		/*
		 
		 Have to cast the returns from ofxXmlsettings::getValue for some reason
		 even though we pass in a default value to make it call the correct
		 function.
		 
		 */
		if(propType == "float"){
			_appModel->setProperty(propName,
								   (float)(_xml->getValue("property",(float)0.0f, i)));
		}
		else if(propType == "int"){
			_appModel->setProperty(propName,
								   (int)(_xml->getValue("property",(int)1, i)));
		}
		else if(propType == "bool"){
			_appModel->setProperty(propName,
								   (bool)(_xml->getValue("property",defaultString, i) == "true" ? true : false));
		}
		else if(propType == "string"){
			_appModel->setProperty(propName,
								   (string)(_xml->getValue("property",defaultString, i)));
		}
		else{
			LOG_WARNING("Could not set property: " + propName + ", unkown type: " + propType);
		}
	} 
	_xml->popTag();
	
	//  close xml file 
	delete xml;
	return true;
}

bool DataController::propertyXMLBuilder(string propertyConfigFilepath){
	int which; // used for ofxXmlSettings
	
	// get list of all properties 
	map<string, string> propsAsMap = _appModel->getAllPropsNameTypeAsMap();

	// set up xml
	ofxXmlSettings xml;
	xml.addTag("config");
	xml.pushTag("config");
	xml.addTag("properties");
	xml.pushTag("properties");

	// create nodes for each property 
	map<string, string>::iterator iter = propsAsMap.begin();
	while(iter != propsAsMap.end()){
		which = xml.addTag("property");
		xml.addAttribute("property", "name", iter->first, which); // name
		xml.addAttribute("property", "type", iter->second, which); // type

		// value
		if(iter->second == "float"){
			xml.setValue("property", boost::any_cast<float>(_appModel->getProperty(iter->first)), which);
		}
		else if(iter->second == "int"){
			xml.setValue("property", boost::any_cast<int>(_appModel->getProperty(iter->first)), which);
		}
		else if(iter->second == "bool"){
			xml.setValue("property", boost::any_cast<bool>(_appModel->getProperty(iter->first)) ? "true" : "false", which);
		}
		else if(iter->second == "string"){
			xml.setValue("property", boost::any_cast<string>(_appModel->getProperty(iter->first)), which);
		}
		else{
			LOG_WARNING("Could not save property: " + iter->first + ", unkown type: " + iter->second);
		}		
		iter++;
	}
	xml.popTag();
	xml.popTag();
	
	// save propertxml
	LOG_NOTICE("Saving properties to xml");
	if(xml.saveFile(propertyConfigFilepath+"_temp.xml")){
		// remove the first file
		remove(ofToDataPath(propertyConfigFilepath, true).c_str());
		// rename temp to final file
		rename(ofToDataPath(propertyConfigFilepath+"_temp.xml", true).c_str(), ofToDataPath(propertyConfigFilepath, true).c_str());
		return true;
	}
	else{
		LOG_ERROR("Could not save properties to xml. File error?");
		return false;
	}
}

bool DataController::sceneXMLParser(string configFilePath){
	
}

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


bool DataController::sceneXMLBuilder(string configFilePath){

	ofxXmlSettings xml; // bulid into this and save it.

	goDirList lister;
	int numFiles;
	map<string, int> filenameToListPosMap;

	// Yuck. have to remember the "which" int that is associated with a 
	// scene name, so we can get back at the right ones to insert details
	// for the scene. Techically, the lister might always return the files
	// correctly grouped, but theres no guarentee to the structure
	map<string, int> nameToXMLWhichMap;
	int which; // used for ofxXmlSettings "which" params
	
	
	vector<string> substrings; // stores split string parts
	
	// add and list the dir
	lister.addDir(ofToDataPath(boost::any_cast<string>(_appModel->getProperty("movieDataPath")), true));
	lister.allowExt("mov");
	lister.allowExt("bin");
	numFiles = lister.listDir(true);
	
	// quick error check
	if(numFiles == 0){
		LOG_ERROR("Could not build scene config, goDirList reported 0 files.");
		abort();
	}
	
	// set up xml basics
	xml.addTag("config:scenes");
	xml.pushTag("config");
	xml.pushTag("scenes"); // set doc root
	
	// Iterate the files 
	for(int fileNumber = 0; fileNumber < numFiles; fileNumber++){
		string filename = lister.getName(fileNumber);
		filename = filename.substr(0, filename.find_last_of(".")); // remove ext
		boost::split(substrings, filename, boost::is_any_of("_")); // split filename
		
		// first substring is the scene name, check if its existant
		if(nameToXMLWhichMap.find(substrings[0]) == nameToXMLWhichMap.end()){
			// doesn't exist, add it
			which = xml.addTag("scene");
			xml.addAttribute("scene", "name", substrings[0], which);
			// Add to info map
			nameToXMLWhichMap.insert(pair<string, int>(substrings[0], which));			
		}

		// get "which" scene tag for this scene name
		which = nameToXMLWhichMap.find(substrings[0])->second;
		// push into scene
		xml.pushTag("scene", which);

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
				which = xml.addTag("sequence");
				xml.setAttribute("sequence", "name", mapName, which);
				// NOTE: this is only a partial creation, does not set all attributes
				nameToXMLWhichMap.insert(pair<string, int>(mapName, which));
			}
			// Find which sequence to push into
			which = nameToXMLWhichMap.find(mapName)->second;
			
			// push into sequence
			xml.pushTag("sequence", which);

			// insert transform stuff
			which = xml.addTag("transform");
			// set attributes for this transform
			xml.addAttribute("transform", "filename", lister.getName(fileNumber), which);

			xml.addAttribute("transform", "size", lister.getSize(fileNumber), which);
			xml.addAttribute("transform", "dateCreated", lister.getCreated(fileNumber), which);
			xml.addAttribute("transform", "dateModified", lister.getModified(fileNumber), which);
			
			xml.popTag(); // pop sequence			
		}
		// loop file for a sequence
		else if(filename.find("loop") != string::npos){
			// does the sequence exist, it might not because we might get a transform
			// before we get a movie, in which case we'll have inserted a basic
			// sequence tag
			string mapName = substrings[1] + "_loop";

			if(nameToXMLWhichMap.find(mapName) == nameToXMLWhichMap.end()){
				// doesn't exist, so create a sequence with the right name
				which = xml.addTag("sequence");
				xml.setAttribute("sequence", "name", mapName, which);
				// NOTE: this is only a partial creation, does not set all attributes
				nameToXMLWhichMap.insert(pair<string, int>(mapName, which));
			}
			which = nameToXMLWhichMap.find(mapName)->second;

			// add the sequence attributes
			xml.setAttribute("sequence", "name", mapName, which);
			xml.setAttribute("sequence", "interactive", "true", which);
			
			// seq01a_loop N-> seq01a_loop
			xml.addAttribute("sequence", "nextSequence", mapName, which);
			
			
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
			xml.addAttribute("sequence", "attackerResult", newName, which);
			newName = "seq"+string(seqNumString)+"b";
			xml.addAttribute("sequence", "victimResult", newName, which);
			
			xml.addAttribute("sequence", "size", lister.getSize(fileNumber), which);
			xml.addAttribute("sequence", "dateCreated", lister.getCreated(fileNumber), which);
			xml.addAttribute("sequence", "dateModified", lister.getModified(fileNumber), which);
		}
		else{
			// Not a loop, not a transform, is a regular sequence movie
			string mapName = substrings[1];

			if(nameToXMLWhichMap.find(mapName) == nameToXMLWhichMap.end()){
				// doesn't exist, so create a sequence with the right name
				which = xml.addTag("sequence");
				xml.setAttribute("sequence", "name", mapName, which);
				// NOTE: this is only a partial creation, does not set all attributes
				nameToXMLWhichMap.insert(pair<string, int>(mapName, which));
			}
			which = nameToXMLWhichMap.find(mapName)->second;
			
			// add the sequence attributes
			xml.setAttribute("sequence", "name", substrings[1], which); // just second part
			xml.setAttribute("sequence", "interactive", "victim", which); // only victim can interact			
			
			// seq01a N-> seq01a_loop
			xml.addAttribute("sequence", "nextSequence", substrings[1]+"_loop", which); // seq01a -> seq01a_loop

			
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
			xml.addAttribute("sequence", "victimResult", newName, which); // seq01a -> seq01b

			xml.addAttribute("sequence", "size", lister.getSize(fileNumber), which);
			xml.addAttribute("sequence", "dateCreated", lister.getCreated(fileNumber), which);
			xml.addAttribute("sequence", "dateModified", lister.getModified(fileNumber), which);
		}
		xml.popTag(); // scene pop 
	}
	
	string filename = boost::any_cast<string>(_appModel->getProperty("scenesXMLFile"));
	if(xml.saveFile(filename+"_temp.xml")){
		// remove the first file
		remove(ofToDataPath(filename, true).c_str());
		// rename temp to final file
		rename(ofToDataPath(filename+"_temp.xml", true).c_str(), ofToDataPath(filename, true).c_str());
		return true;
	}
	else{
		LOG_ERROR("Could not save properties to xml. File error?");
		return false;
	}	
}


void DataController::loadAppProperties(){


}

void DataController::loadSceneData(){
	Scene *scene;
	Sequence *sequence;

	vector<CamTransform> *transform;
	goVideoPlayer *video;
	
	string combinedPath;
	string defaultString = "default string value should not be seen";
	
	bool hasCurrentScene = false; // used to set scene 0 to first current scene
	
	LOG_NOTICE("Loading scene data");
	if(!_xml->tagExists("scenes")){
		LOG_ERROR("No scenes configuration to load!");
		abort();
	}
	
	// set scenes to root
	_xml->pushTag("scenes");
	
	for(int sceneNum = 0; sceneNum < _xml->getNumTags("scene"); sceneNum++){
		/*
		 debated pushTag(scene) here but then we'd be 
		 doing get attribute without a tag which would be odd
		*/
		if(!_xml->attributeExists("scene", "name", sceneNum)){
			LOG_ERROR("Could not set scene name, no name?");
		}
		
		// make new scene
		scene = new Scene();
		
		// save name
		scene->setName(_xml->getAttribute("scene", "name", defaultString, sceneNum));
		
		// convenience
		string sceneRootPath = ofToDataPath((boost::any_cast<string>)(_appModel->getProperty("movieDataPath"))) + "/" + scene->getName() + "/";
				
		/*
			push into scene so we can check attributes of sequences,
			cant just do it regularly because we have two "which" params,
			which scene and which sequence
		*/
		_xml->pushTag("scene", sceneNum);
		
		// find the children of the scene node (ie: sequences)		
		for(int seqNum = 0; seqNum < _xml->getNumTags("sequence"); seqNum++){
			
			// create sequence
			sequence = new Sequence();

			// set name
			if(!_xml->attributeExists("sequence", "name", seqNum)){
				LOG_ERROR("No sequence name for sequence: "+ofToString(seqNum));
				abort();
			}
			sequence->setName(_xml->getAttribute("sequence", "name", defaultString, seqNum));
			
			// check for interactive attribute (loop videos wait for interaction)
			if(_xml->attributeExists("sequence", "interactive", seqNum)){
				LOG_VERBOSE(sequence->getName()+ " has interactive attribute");
				sequence->setIsInteractive(true);
				if(!_xml->attributeExists("sequence", "victimResult", seqNum) ||
				   !_xml->attributeExists("sequence", "attackerResult", seqNum)){
					LOG_ERROR("Sequence " + sequence->getName() + " is interactive but has no victim/attacker result names");
					delete sequence;
					abort();
				}
				// set attacker/victim results
				sequence->setVictimResult(_xml->getAttribute("sequence", "victimResult", defaultString, seqNum)); 
				sequence->setAttackerResult(_xml->getAttribute("sequence", "attackerResult", defaultString, seqNum)); 
			}
			else{
				LOG_VERBOSE(sequence->getName()+ " does not have interactive attribute");
				sequence->setIsInteractive(false);
			}
			
			// set up next sequence name
			if(_xml->attributeExists("sequence", "nextSequence", seqNum)){
				sequence->setNextSequenceName(_xml->getAttribute("sequence", "nextSequence", defaultString, seqNum));
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
			_xml->pushTag("sequence", seqNum);
			for(int transNum = 0; transNum < _xml->getNumTags("transform"); transNum++){
				transform = new vector<CamTransform>();
				if(!loadVector<CamTransform>(sceneRootPath+_xml->getAttribute("transform", "filename", defaultString, transNum),
											 transform)){
					// load Vector failed will log own error
					delete transform;
					abort();
				}
				// insert transform into sequence vector
				sequence->addTransform(*transform);				
			}
			_xml->popTag(); // pop out of sequence
			
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

			// made the sequence, insert it into scene
			scene->setSequence(sequence->getName(), sequence);
			if(seqNum == 0){
				// set first sequence to current sequence
				scene->setCurrentSequence(sequence->getName());
			}

		} // end sequence for loop
		
		_xml->popTag(); // pop out of scene
		
		// finished creation, insert
		_appModel->setScene(scene->getName(), scene);
		if(sceneNum == 0){
			// set first scene to current scene
			_appModel->setCurrentScene(scene->getName());
		}
		
	} // end scene for loop

	_xml->popTag(); // pop out of scenes
}

template <class vectorType>
bool DataController::loadVector(string filePath, vector< vectorType > * vec) {
	LOG_NOTICE("Load vector:: " + filePath);
	std::ifstream ifs(filePath.c_str());
	if(ifs.fail()){
		LOG_ERROR("Could not load vector: " + filePath);
		abort(); // Could be a bit over zealous
	}
    boost::archive::text_iarchive ia(ifs);
    ia >> (*vec);
	return true;
}
