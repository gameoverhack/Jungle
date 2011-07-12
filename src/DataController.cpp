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

	propertyXMLParser(configFilePath);
	propertyXMLBuilder(configFilePath);

	SceneXMLBuilder sceneBuilder(boost::any_cast<string>(_appModel->getProperty("scenesDataPath")),
					boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));

	sceneBuilder.santiseFiles();
	sceneBuilder.scanFiles();
	sceneBuilder.build();
	sceneBuilder.save();

	sceneXMLParser(boost::any_cast<string>(_appModel->getProperty("scenesXMLFile")));
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController(){
}


bool DataController::propertyXMLParser(string propertyConfigFilepath){
	//  load up the property xml file 
	ofxXmlSettings xml;
	if(!xml.loadFile(propertyConfigFilepath)){
		LOG_ERROR("Could not load property config: " + propertyConfigFilepath);
		abort();		
	}
	
	xml.pushTag("config"); // push into root.
	//  pull out the properties 
	string defaultString = "this should never be seen"; // getValue requires type to know what to return
	string propName, propValue, propType;
	
	LOG_NOTICE("Discovering properties");
	// Find all the property tags
	if(!xml.tagExists("properties")){
		LOG_ERROR("No properties node in config");
		abort();
	}
	xml.pushTag("properties"); // set properties as root node
	
	// loop over all property tags
	for(int i = 0; i < xml.getNumTags("property"); i++){
		propName = xml.getAttribute("property",
									  "name", defaultString, i);
		propType = xml.getAttribute("property",
									  "type", defaultString, i);
		
		/*
		 
		 Have to cast the returns from ofxXmlsettings::getValue for some reason
		 even though we pass in a default value to make it call the correct
		 function.
		 
		 */
		if(propType == "float"){
			_appModel->setProperty(propName,
								   (float)(xml.getValue("property",(float)0.0f, i)));
		}
		else if(propType == "int"){
			_appModel->setProperty(propName,
								   (int)(xml.getValue("property",(int)1, i)));
		}
		else if(propType == "bool"){
			_appModel->setProperty(propName,
								   (bool)(xml.getValue("property",defaultString, i) == "true" ? true : false));
		}
		else if(propType == "string"){
			_appModel->setProperty(propName,
								   (string)(xml.getValue("property",defaultString, i)));
		}
		else{
			LOG_WARNING("Could not set property: " + propName + ", unkown type: " + propType);
		}
	} 
	xml.popTag();

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

bool DataController::sceneXMLParser(string sceneConfigFilePath){
	Scene *scene;
	Sequence *sequence;
	
	vector<CamTransform> *transform;
	goVideoPlayer *video;
	
	string combinedPath;
	string defaultString = "default string value should not be seen";
	
	bool hasCurrentScene = false; // used to set scene 0 to first current scene
	
	ofxXmlSettings xml;
	if(!xml.loadFile(sceneConfigFilePath)){
		LOG_ERROR("Could not load scene config: " + sceneConfigFilePath);
		abort();		
	}
	
	xml.pushTag("config"); // move into root
	
	LOG_NOTICE("Loading scene data");
	if(!xml.tagExists("scenes")){
		LOG_ERROR("No scenes configuration to load!");
		abort();
	}
	
	// set scenes to root
	xml.pushTag("scenes");
	
	for(int sceneNum = 0; sceneNum < xml.getNumTags("scene"); sceneNum++){
		/*
		 debated pushTag(scene) here but then we'd be 
		 doing get attribute without a tag which would be odd
		 */
		if(!xml.attributeExists("scene", "name", sceneNum)){
			LOG_ERROR("Could not set scene name, no name?");
		}
		
		// make new scene
		scene = new Scene();
		
		// save name
		scene->setName(xml.getAttribute("scene", "name", defaultString, sceneNum));
		
		// convenience
		string sceneRootPath = ofToDataPath((boost::any_cast<string>)(_appModel->getProperty("movieDataPath"))) + "/" + scene->getName() + "/";
		
		/*
		 push into scene so we can check attributes of sequences,
		 cant just do it regularly because we have two "which" params,
		 which scene and which sequence
		 */
		xml.pushTag("scene", sceneNum);
		
		// find the children of the scene node (ie: sequences)		
		for(int seqNum = 0; seqNum < xml.getNumTags("sequence"); seqNum++){
			
			// create sequence
			sequence = new Sequence();
			
			// set name
			if(!xml.attributeExists("sequence", "name", seqNum)){
				LOG_ERROR("No sequence name for sequence: "+ofToString(seqNum));
				abort();
			}
			sequence->setName(xml.getAttribute("sequence", "name", defaultString, seqNum));
			
			// check for interactive attribute (loop videos wait for interaction)
			string iteractivityType = xml.getAttribute("sequence", "interactive", defaultString, seqNum);
			if(iteractivityType == "both"){
				LOG_VERBOSE(sequence->getName()+ " has interactive attribute");
				sequence->setIsInteractive(true);
				if(!xml.attributeExists("sequence", "victimResult", seqNum) ||
				   !xml.attributeExists("sequence", "attackerResult", seqNum)){
					LOG_ERROR("Sequence " + sequence->getName() + " is interactive but has no victim/attacker result names");
					delete sequence;
					abort();
				}
				// set attacker/victim results
				sequence->setVictimResult(xml.getAttribute("sequence", "victimResult", defaultString, seqNum)); 
				sequence->setAttackerResult(xml.getAttribute("sequence", "attackerResult", defaultString, seqNum)); 
			}
			else{
				LOG_VERBOSE(sequence->getName()+ " does not have 'both' interactive flag. MUST ADD PROPERTIES TO SEQUENCE TO HANDLE 'face' and 'victim (only)'");
				sequence->setIsInteractive(false);
			}
			
			// set up next sequence name
			if(xml.attributeExists("sequence", "nextSequence", seqNum)){
				sequence->setNextSequenceName(xml.getAttribute("sequence", "nextSequence", defaultString, seqNum));
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
			xml.pushTag("sequence", seqNum);
			for(int transNum = 0; transNum < xml.getNumTags("transform"); transNum++){
				transform = new vector<CamTransform>();
				if(!loadVector<CamTransform>(sceneRootPath+xml.getAttribute("transform", "filename", defaultString, transNum),
											 transform)){
					// load Vector failed will log own error
					delete transform;
					abort();
				}
				// insert transform into sequence vector
				sequence->addTransform(*transform);				
			}
			xml.popTag(); // pop out of sequence
			
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
		
		xml.popTag(); // pop out of scene
		
		// finished creation, insert
		_appModel->setScene(scene->getName(), scene);
		if(sceneNum == 0){
			// set first scene to current scene
			_appModel->setCurrentScene(scene->getName());
		}
		
	} // end scene for loop
	
	xml.popTag(); // pop out of scenes
	
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
