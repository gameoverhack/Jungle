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
	
	loadAppProperties();
	loadSceneData();
	
	LOG_NOTICE("Initialisation complete");
}

DataController::~DataController() {
	delete _xml;
}

void DataController::loadAppProperties(){
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
