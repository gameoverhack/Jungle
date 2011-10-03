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
	load();
	setupFileListers();

	parseXML();
}

void SceneXMLParser::setupFileListers(){
	_moviesFileLister.reset();
	_moviesFileLister.addDir(_dataPath);
	_moviesFileLister.allowExt("mov");
	_moviesFileLister.allowExt("MOV");
	_moviesFileLister.listDir(true);


	_assetsFileLister.reset();
	_assetsFileLister.addDir(_dataPath);
	_assetsFileLister.allowExt("bin");
	_assetsFileLister.allowExt("BIN");
	_assetsFileLister.listDir(true);

	// quick error check
	if(_moviesFileLister.size() == 0){
		LOG_ERROR("Could not load scene config, goDirList reported 0 movie files found.");
		throw JungleException("0 movie files found");
	}

	if(_assetsFileLister.size() == 0){
		LOG_ERROR("Could not load scene config, goDirList reported 0 asset files found.");
		throw JungleException("0 asset files found");
	}
}

void SceneXMLParser::parseXML(){
	vector<string> attributesToCheck;
	vector<CamTransform> *transform = NULL;
	int fileId;
	string fileName;
	char int2str[512];
	vector<string> brokenfiles;

	Scene *scene;
	Sequence *sequence;

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

	// sanity check
	if(_xml.getNumTags("scene") == 0){
		LOG_ERROR("No scene nodes in xml");
		throw GenericXMLParseException("No scene nodes to xml");
	}

	// push into each scene node
	for(int whichScene = 0; whichScene < _xml.getNumTags("scene"); whichScene++){
		// check attributes
		attributesToCheck.clear();
		attributesToCheck.push_back("name");
		checkTagAttributesExist("scene", attributesToCheck, whichScene);

		scene = new Scene();
		scene->setName(_xml.getAttribute("scene", "name", "", whichScene));
		scene->setTotalFrames(_xml.getAttribute("scene", "totalFrames", -1, whichScene)); // no loop or b seq's

		LOG_VERBOSE("Parsing xml=>scene: " + scene->getName());
		// set new root
		_xml.pushTag("scene", whichScene);

		// sanity check
		if(_xml.getNumTags("sequence") == 0){
			LOG_ERROR("No sequence nodes for scene in xml");
			throw GenericXMLParseException("No sequence nodes for scene in xml");
		}
		// iterate over all the sequences
		for(int whichSequence = 0; whichSequence < _xml.getNumTags("sequence"); whichSequence++){

			// check attributes
			attributesToCheck.clear();
			attributesToCheck.push_back("name");
			attributesToCheck.push_back("size");
			attributesToCheck.push_back("frames");
			attributesToCheck.push_back("dateModified");
			attributesToCheck.push_back("dateCreated");
			attributesToCheck.push_back("nextSequence");
			attributesToCheck.push_back("faceResult");
			attributesToCheck.push_back("attackerResult");
			attributesToCheck.push_back("victimResult");
			attributesToCheck.push_back("faked");
			attributesToCheck.push_back("sequenceType");
			attributesToCheck.push_back("filename");
			checkTagAttributesExist("sequence", attributesToCheck, whichSequence);

			sequence = new Sequence();

			sequence->setName(_xml.getAttribute("sequence", "name", "", whichSequence));
			LOG_VERBOSE("Parsing xml=>sequence: " + sequence->getName());
			sequence->setType(_xml.getAttribute("sequence", "sequenceType", "", whichSequence));
			sequence->setFaceResult(_xml.getAttribute("sequence", "faceResult", "", whichSequence));
			sequence->setAttackerResult(_xml.getAttribute("sequence", "attackerResult", "", whichSequence));
			sequence->setVictimResult(_xml.getAttribute("sequence", "victimResult", "", whichSequence));
			sequence->setNextSequenceName(_xml.getAttribute("sequence", "nextSequence", "", whichSequence));
			sequence->setIsSequenceFaked((_xml.getAttribute("sequence", "faked", "", whichSequence) == "true") ? true : false);
			sequence->setNumber(findSequenceNumberFromString(sequence->getName()));

			// check movie file metadata
			fileName = _xml.getAttribute("sequence", "filename", "", whichSequence);
			fileId = _moviesFileLister.findFileByName(fileName);

			// force file date time create/modified to actual file create/modified dates in case of copying...
			if (fileId != -1 && boost::any_cast<bool>(_appModel->getProperty("xmlForceFileDateTime"))) {
                forceTagAttribute("sequence", "dateCreated", _moviesFileLister.getCreated(fileId), whichSequence);
                forceTagAttribute("sequence", "dateModified", _moviesFileLister.getModified(fileId), whichSequence);
			}

			if(fileId == -1 ||
			   !compareTagAttribute("sequence", "dateCreated", _moviesFileLister.getCreated(fileId), whichSequence) ||
			   !compareTagAttribute("sequence", "dateModified", _moviesFileLister.getModified(fileId), whichSequence) ||
			   !compareTagAttribute("sequence", "size", boost::lexical_cast<std::string>(_moviesFileLister.getSize(fileId)), whichSequence)){
				LOG_VERBOSE("Comparision failure or file missing on " + fileName);
				throw GenericXMLParseException("Missing movie file");
			}

			sequence->setMovieFullFilePath(_moviesFileLister.getPath(fileId));
            sequence->setNumFrames(_xml.getAttribute("sequence", "frames", -1, whichSequence));

			// push in
			_xml.pushTag("sequence", whichSequence);

			// sanity check
			if(_xml.getNumTags("interactivity") == 0 || _xml.getNumTags("transform") == 0 ){
				LOG_ERROR("No interactivity or transform nodes in xml for " + sequence->getName());
				throw GenericXMLParseException("No interactivity or transform nodes in xml for " + sequence->getName());
			}

			// interactivity
			attributesToCheck.clear();
			attributesToCheck.push_back("filename");
			attributesToCheck.push_back("size");
			attributesToCheck.push_back("dateCreated");
			attributesToCheck.push_back("dateModified");
			checkTagAttributesExist("interactivity", attributesToCheck, 0);

			fileName = _xml.getAttribute("interactivity", "filename", "", 0);
			fileId = _assetsFileLister.findFileByName(fileName);

            // force file date time create/modified to actual file create/modified dates in case of copying...
			if (fileId != -1 && boost::any_cast<bool>(_appModel->getProperty("xmlForceFileDateTime"))) {
			   forceTagAttribute("interactivity", "dateCreated", _assetsFileLister.getCreated(fileId), 0);
			   forceTagAttribute("interactivity", "dateModified", _assetsFileLister.getModified(fileId), 0);
			}

			if(fileId == -1 ||
			   !compareTagAttribute("interactivity", "dateCreated", _assetsFileLister.getCreated(fileId), 0) ||
			   !compareTagAttribute("interactivity", "dateModified", _assetsFileLister.getModified(fileId), 0) ||
			   !compareTagAttribute("interactivity", "size", boost::lexical_cast<std::string>(_assetsFileLister.getSize(fileId)), 0)){
				LOG_VERBOSE("Comparision failure on interactivity for " + sequence->getName());
				throw GenericXMLParseException("Interactivity file was missing of invalid");
//				brokenfiles.push_back(_xml.getAttribute("interactivity", "filename", "", 0));
			} else {
				LOG_VERBOSE("Adding interactivity for " + _assetsFileLister.getName(fileId));
				// Load up stuff into the sequence
				SequenceDescriptor *descriptor = new SequenceDescriptor();

				// find and load the file
				loadClass(_assetsFileLister.getPath(fileId), descriptor);
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
					LOG_VERBOSE("FACE FramePair: " + ofToString(fp._start) + " to " + ofToString(fp._end));
					for(int i = fp._start; i < fp._end; i++){
						interactionTable[i] = kINTERACTION_FACE;
					}
				}

				// set up victim
				for(vecIter = descriptor->_victim.begin(); vecIter != descriptor->_victim.end(); vecIter++){
					FramePair fp = *vecIter;
					LOG_VERBOSE("VIC  FramePair: " + ofToString(fp._start) + " to " + ofToString(fp._end));
					for(int i = fp._start; i < fp._end; i++){
						interactionTable[i] = kINTERACTION_VICTIM;
					}
				}

				// set up attacker
				for(vecIter = descriptor->_attacker.begin(); vecIter != descriptor->_attacker.end(); vecIter++){
					FramePair fp = *vecIter;
					LOG_VERBOSE("ATK  FramePair: " + ofToString(fp._start) + " to " + ofToString(fp._end));
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

			}

			// transforms
			for(int witchTransform = 0; witchTransform < _xml.getNumTags("transform"); witchTransform++){

				fileName = _xml.getAttribute("transform", "filename", "", witchTransform);
				fileId = _assetsFileLister.findFileByName(fileName);

                // force file date time create/modified to actual file create/modified dates in case of copying...
                if (fileId != -1 && boost::any_cast<bool>(_appModel->getProperty("xmlForceFileDateTime"))) {
                   forceTagAttribute("transform", "dateCreated", _assetsFileLister.getCreated(fileId), witchTransform);
				   forceTagAttribute("transform", "dateModified", _assetsFileLister.getModified(fileId), witchTransform);
                }

				if(fileId == -1 ||
				   !compareTagAttribute("transform", "dateCreated", _assetsFileLister.getCreated(fileId), witchTransform) ||
				   !compareTagAttribute("transform", "dateModified", _assetsFileLister.getModified(fileId), witchTransform) ||
				   !compareTagAttribute("transform", "size", boost::lexical_cast<std::string>(_assetsFileLister.getSize(fileId)), witchTransform)){
					LOG_VERBOSE("Comparision failure on transform for " + sequence->getName());
					throw GenericXMLParseException("Transform file was missing or invalid");
//					brokenfiles.push_back(_xml.getAttribute("transform", "filename", "", witchTransform));
				} else {
					// Load up transforms into the sequence
					transform = new vector<CamTransform>();
					loadVector(_assetsFileLister.getPath(fileId), transform);
					string filename = _assetsFileLister.getNameWithoutExtension(fileId);
					string name = filename.substr(filename.find_last_of("_")+1);
					sequence->setTransform(name, transform);
					LOG_VERBOSE("Adding transform for " + name);
				}
			}

			_xml.popTag(); // pop out of sequence

			// add the scene
			scene->setSequence(sequence->getName(), sequence);

			if(whichSequence == 0){
				// first sequence, set it to the 'current sequence' in the scene
				scene->setCurrentSequence(sequence->getName());
			}

		} // end sequence tag for loop
		_xml.popTag(); // pop out of scene
		_appModel->setScene(scene->getName(), scene); // add scene
		if(whichScene == 0){
			// first scene, set it to be the 'current scene'
			_appModel->setCurrentScene(scene->getName());
		}
	} // end scene tag for loop

	// check if we had broken files
//	if(brokenfiles.size() != 0){
//		// reset app model because its invalid
//		_appModel->clearScenesAndSequences();
//		// throw that up the broken files
//		throw AnalysisRequiredException("Analysis required", brokenfiles);
//	}

}

void SceneXMLParser::checkTagAttributesExist(string xmltag, vector<string> attributes, int which) {
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

bool SceneXMLParser::compareTagAttribute(string xmltag, string attribute, string target, int which) {

	vector<string> attributesToCheck;
	attributesToCheck.push_back(attribute);
	checkTagAttributesExist(xmltag, attributesToCheck, which);
	string value = _xml.getAttribute(xmltag, attribute, "1@#$21238(&$*&^@#*^saltsaltsaltsalt", which);

	LOG_VERBOSE("Comparing: [value] " + value + " == [target] " + target + (string)(value == target ? " TRUE" : " FALSE"));

	return (value == target);

}

void SceneXMLParser::forceTagAttribute(string xmltag, string attribute, string target, int which) {

    LOG_WARNING("Forcing " + xmltag + "::" + attribute + " to " + target);

	vector<string> attributesToForce;
	attributesToForce.push_back(attribute);
	checkTagAttributesExist(xmltag, attributesToForce, which);
	_xml.setAttribute(xmltag, attribute, target, which);

}
