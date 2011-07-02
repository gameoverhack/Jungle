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

	/* try to open the file */
	_xmldoc = new TiXmlDocument(configFilePath);	
	if(!_xmldoc->LoadFile()){ /* Capital letter method names wat. */
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

void DataController::loadAppProperties(string fs){
	TiXmlElement *el;	
	string propName, propValue, propType;

	/* find the properties node */
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
	string transformFilename;
	ofxAlphaVideoPlayer *video;
	
	LOG_NOTICE("Loading scene data");
	
	/* find the scenes */
	el = _xmldoc->RootElement()->FirstChildElement("scenes");
	for(TiXmlElement *sceneEl = el->FirstChildElement("scene");
			sceneEl; sceneEl = sceneEl->NextSiblingElement()){

		/* make new scene */
		scene = new Scene();
		/* try to get attribute */
		if(!sceneEl->Attribute("name")){
			LOG_ERROR("Could not set scene name, no name? ");
			delete scene;
			continue;
		}

		/* save name */
		scene->setName(sceneEl->Attribute("name"));

		/* find the children of the scene node (ie: sequences) */
		bool hasSetCurrentSequence, hasSetCurrentScene;
		for(TiXmlElement *seqEl = sceneEl->FirstChildElement("sequence");
				seqEl; seqEl = seqEl->NextSiblingElement("sequence")){

			sequence = new Sequence();
			sequence->setName(seqEl->Attribute("name"));
			sequence->setVictimResult(seqEl->Attribute("victimResult"));
			sequence->setAttackerResult(seqEl->Attribute("attackerResult"));

			/* find sequence movie */
			TiXmlElement *mvSeq = seqEl->FirstChildElement("sequenceMovie");
			/* set up sequence movie */
			video = new ofxAlphaVideoPlayer();
			video->loadMovie(mvSeq->Attribute("filename")); /* TODO: error check this attrbute call */
			sequence->setSequenceMovie(video);
			
			/* find the sequence transforms */
			for(TiXmlElement *transEl = mvSeq->FirstChildElement("transform");
				transEl; transEl = transEl->NextSiblingElement("transform")){
				transform = new vector<CamTransform>();
				if(!loadVector<CamTransform>(ofToDataPath(transEl->Attribute("filename")), transform)){
					delete transform;
					continue;
				}
				/* insert transform into sequence vector */
				sequence->addSequenceTransform(*transform);
			}
			
			
			
			
			/* find loop movie */
			
			/* find the transforms */
			for(TiXmlElement *transEl = seqEl->FirstChildElement("transform");
					transEl; transEl = transEl->NextSiblingElement("transform")){
				transform = new vector<CamTransform>();
				if(!loadVector<CamTransform>(ofToDataPath(transEl->Attribute("filename")), transform)){
					delete transform;
					continue;
				}
				/* insert transform into sequence vector */
				sequence->addTransform(*transform);
			}
			/* made the sequence, insert it into scene */
			scene->setSequence(sequence->getName(), sequence);
			if(!hasSetCurrentSequence){
				scene->setCurrentSequence(sequence->getName());
			}
		}
		_appModel->setScene(scene->getName(), scene);
		if(!hasSetCurrentScene){
			_appModel->setCurrentScene(scene->getName());
		}
	}
	
	_appModel->getCurrentSequence()->_sequenceVideo.loadMovie("/Users/ollie/Source/of_62_osx/apps/stranger_danger_artifacts/t_seq_01_all_alpha_embedded2.mov");
	_appModel->getCurrentSequence()->_sequenceVideo.play();	
}

template <class vectorType>
bool DataController::loadVector(string filePath, vector< vectorType > * vec) {
	LOG_NOTICE("Load: " + filePath);
	ofLog(OF_LOG_VERBOSE, ("LoadVector: " + filePath));
	std::ifstream ifs(filePath.c_str());
	if(ifs.fail()){
		LOG_ERROR("Could not load " + filePath);
		abort(); /* Could be a bit over zealous */
	}
    boost::archive::text_iarchive ia(ifs);
    ia >> (*vec);
	return true;
}
