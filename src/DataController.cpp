/*
 *  DataController.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DataController.h"
#include "tinyxml.h"

DataController::DataController(string configFilePath){
	LOG_NOTICE("Initialising with " + configFilePath);

	// try to open the file
	_xmldoc = new TiXmlDocument(configFilePath);	
	if(!_xmldoc->LoadFile()){ // Capital letter method names wat.
		LOG_ERROR("Could not load config: " + configFilePath);
		/*
		 Not sure how we should be failing here, no global quit method in the app
		 Don't want to assert because we want to log the failure.
		 */
		abort();
	}
	
	loadAppProperties(configFilePath);
	loadSceneData(configFilePath);
	LOG_NOTICE("Initialisation complete");
	
}

DataController::~DataController(){
	delete _xmldoc;
}

void DataController::loadAppProperties(string fs){
	TiXmlElement *el;	
	string propName, propValue, propType;

	// find the properties node
	LOG_NOTICE("Discovering properties");
	el = _xmldoc->RootElement()->FirstChildElement("properties");
	for(TiXmlElement *prop = el->FirstChildElement(); prop; prop = prop->NextSiblingElement()){
		LOG_NOTICE("Reading property: " + string(prop->Value())  +
				   "(" + string(prop->Attribute("type")) +
				   ")=> " + string(prop->FirstChild()->Value()));
		propName = string(prop->Value());
		propType = string(prop->Attribute("type"));
		propValue = string(prop->FirstChild()->Value());
		
		if(propType == "float"){
			_appModel->setProperty(propName, strtof(propValue.c_str(), NULL));
		}
		else if(propType == "int"){
			_appModel->setProperty(propName, strtol(propValue.c_str(), NULL, 10));
		}
		else if(propType == "bool"){
			_appModel->setProperty(propName, (propValue == "true" ? true : false));
		}
		else if(propType == "string"){
			_appModel->setProperty(propName, propValue);
		}
		else{
			LOG_WARNING("Could not set property: " + propName + ", unkown type: " + propType);
		}
	}
}

void DataController::loadSceneData(string filePath){
	TiXmlElement *el;

	Scene *scene;
	Sequence *sequence;

	vector<CamTransform> *transform;
	goVideoPlayer *video;
	
	string combinedPath;
	
	LOG_NOTICE("Loading scene data");
	
	// find the scenes
	bool hasSetCurrentScene = false;
	el = _xmldoc->RootElement()->FirstChildElement("scenes");
	for(TiXmlElement *sceneEl = el->FirstChildElement("scene");
			sceneEl; sceneEl = sceneEl->NextSiblingElement()){

		// make new scene
		scene = new Scene();
		// try to get attribute
		if(!sceneEl->Attribute("name")){
			LOG_ERROR("Could not set scene name, no name? ");
			delete scene;
			continue;
		}

		// save name
		scene->setName(sceneEl->Attribute("name"));
		
		// convenience
		string sceneRootPath = ofToDataPath((boost::any_cast<string>)(_appModel->getProperty("movieDataPath"))) + "/" + scene->getName() + "/";
		
		bool hasSetCurrentSequence = false;
		// find the children of the scene node (ie: sequences)		
		for(TiXmlElement *seqEl = sceneEl->FirstChildElement("sequence");
				seqEl; seqEl = seqEl->NextSiblingElement("sequence")){

			sequence = new Sequence();
			sequence->setName(seqEl->Attribute("name"));

			// check if this has interactive attribute
			if(seqEl->Attribute("interactive")){
				LOG_VERBOSE(sequence->getName()+ " has interactive attribute");
				sequence->setIsInteractive(true);
				sequence->setVictimResult(seqEl->Attribute("victimResult")); 	// todo: error checking on existance of these attributes
				sequence->setAttackerResult(seqEl->Attribute("attackerResult"));// todo: error checking on existance of these attributes				
			}
			else{
				LOG_VERBOSE(sequence->getName()+ " does not have interactive attribute");
				sequence->setIsInteractive(false);
			}

			// setup next sequence stuff
			if(seqEl->Attribute("nextSequence")){
				sequence->setNextSequenceName(seqEl->Attribute("nextSequence"));
				LOG_VERBOSE("Setting next sequence to: " + sequence->getNextSequenceName());
			}
			else{
				if(!sequence->getIsInteractive()){
					// seq isnt interactive, so no nextSequence attribute means its the final sequence
					sequence->setNextSequenceName(kLAST_SEQUENCE_TOKEN);
					LOG_VERBOSE("Setting next sequence to: " + kLAST_SEQUENCE_TOKEN);
				}
				else{
					// just to make sure its set to something, techinically, a "no
					//  next sequence" sequence loops, so it is its own next sequence				
					sequence->setNextSequenceName(seqEl->Attribute("name"));
				}
			}
			
			
			
			// Find the transforms for this sequence
			for(TiXmlElement *transEl = seqEl->FirstChildElement("transform");
				transEl; transEl = transEl->NextSiblingElement("transform")){
				transform = new vector<CamTransform>();
				if(!loadVector<CamTransform>(sceneRootPath+transEl->Attribute("filename"), transform)){
					// load Vector failed will log own error
					delete transform;
					//continue;
					abort();
				}
				// insert transform into sequence vector
				sequence->addTransform(*transform);
			}
			
			// Load the sequence movie
			video = new goVideoPlayer();
			combinedPath = sceneRootPath+scene->getName() + "_" + sequence->getName()+".mov";
			if(!video->loadMovie(combinedPath)){
				LOG_ERROR("Could not load movie: "+combinedPath);
				delete video; // free video
				delete sequence; // cant use sequence without video so fail it
				//continue;
				abort(); // lets just assume no video for one means whole thing is broken
			};
			sequence->setSequenceMovie(video);

			// made the sequence, insert it into scene
			scene->setSequence(sequence->getName(), sequence);
			if(!hasSetCurrentSequence){
				// set first sequence to current sequence
				scene->setCurrentSequence(sequence->getName());
				hasSetCurrentSequence = true;
			}
		}
		
		// finished creation, insert
		_appModel->setScene(scene->getName(), scene);
		if(!hasSetCurrentScene){
			// set first scene to current scene
			_appModel->setCurrentScene(scene->getName());
			hasSetCurrentScene = true;
		}
	}
}

template <class vectorType>
bool DataController::loadVector(string filePath, vector< vectorType > * vec) {
	LOG_NOTICE("Load: " + filePath);
	ofLog(OF_LOG_VERBOSE, ("LoadVector: " + filePath));
	std::ifstream ifs(filePath.c_str());
	if(ifs.fail()){
		LOG_ERROR("Could not load " + filePath);
		abort(); // Could be a bit over zealous
	}
    boost::archive::text_iarchive ia(ifs);
    ia >> (*vec);
	return true;
}
