/*
 *  Analyzer.cpp
 *  Jungle
 *
 *  Created by gameover on 4/08/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "Analyzer.h"

//--------------------------------------------------------------
Analyzer::Analyzer() {
	
}

//--------------------------------------------------------------
Analyzer::~Analyzer() {
	
}

//--------------------------------------------------------------
void Analyzer::registerStates() {
	registerState(kANAL_READY, "kANAL_READY");
	registerState(kANAL_CONNECTING, "kANAL_CONNECTING");
	registerState(kANAL_SERIALIZE, "kANAL_SERIALIZE");
	registerState(kANAL_IGNORE, "kANAL_IGNORE");
	registerState(kANAL_FINISHED, "kANAL_FINISHED");
	registerState(kANAL_ERROR, "kANAL_ERROR");
	
	setState(kANAL_READY);
}

//--------------------------------------------------------------
void Analyzer::setup(vector<string> * files, int port) {

	LOG_NOTICE("Setting up the ANALyzer");
	
	LOG_NOTICE("Given " + ofToString((int)files->size()) + " names. Checking for duplicates...");
	
	
	// there must be an easier cleaner way but the combination of flash boringness and my frustration makes me do this peice of horribleness...
	
	for (int i = 0; i < files->size(); i++) {
		
		vector<string> nameSplit = ofSplitString(files->at(i), ":");
		
		string fileName = nameSplit[0];
		string sceneName = nameSplit[1];
		string typeName;
		
		if (nameSplit.size() > 2) {
			typeName = nameSplit[2];
		} else typeName = "";
		
		FileScenes * thisFileScene;
		
		if (_sFiles.count(fileName) == 0) {
			
			_sFiles.insert(fileName);
			FileScenes * f = new FileScenes;
			f->_fileName = fileName;
			f->_processedScenes = 0;
			_files.push_back(f);
			thisFileScene = f;
			
		} else {
			
			for (int j = 0; j < _files.size(); j++) {
				if (_files[j]->_fileName == fileName) {
					thisFileScene = _files[j];
					break;
				}
			}
			
		}

		
		if (thisFileScene->_sScenes.count(fileName + "_" + sceneName) == 0) {
			thisFileScene->_sScenes.insert(fileName + "_" + sceneName);
			thisFileScene->_scenes.push_back(sceneName);
		}
		
		map<string, string>::iterator it;
		
		it = thisFileScene->_mTypes.find(fileName + "_" + sceneName);
		if (it == thisFileScene->_mTypes.end()) {
			thisFileScene->_mTypes.insert(pair<string, string>(fileName + "_" + sceneName, typeName));
		} else if (typeName != "") {
			string allTypes = it->second + "-" + typeName;
			thisFileScene->_mTypes[fileName + "_" + sceneName] = it->second + "-" + typeName;
		}
		
	}
	
	LOG_NOTICE("...actually checking ");
	
	for (int i = 0; i < _files.size(); i++) {
		LOG_NOTICE("File: " + _files[i]->_fileName + " with scenes: ");
		for (int j = 0; j < _files[i]->_scenes.size(); j++) {
			LOG_NOTICE("	" + _files[i]->_scenes[j]);
			LOG_NOTICE("		types: " + _files[i]->_mTypes[_files[i]->_fileName + "_" + _files[i]->_scenes[j]]);
		}
	}
	
	_processedFiles = 0;
	
	//ofSetLogLevel(OF_LOG_VERBOSE);
	
	if(!_flexComManager.setup(port)) {
		LOG_ERROR("Could not setup server on port, or something similiar");
		abort();
	} else {
		LOG_NOTICE("FlexCOM setup ok");
	}

	
	ofAddListener(_flexComManager.messageReceived, this, &Analyzer::serializeMessage);
	setState(kANAL_CONNECTING);
	
}

//--------------------------------------------------------------
void Analyzer::update() {
	
	_flexComManager.update();
	
	if (_flexComManager.checkState(LOAD) && _processedFiles < _files.size()) {
		
#ifdef TARGET_OSX
		string systemMsg = "open ";
		string flashPath = boost::any_cast<string>(_appModel->getProperty("flashDataPath"));
		string filePath = "'" + flashPath + "/" + _files[_processedFiles]->_fileName + ".app" + "'";
		
		systemMsg += filePath;
		
		LOG_NOTICE("Attempting to Load: " + filePath);
		
		int err = system(systemMsg.c_str());
		
		if(err != 0) { ////"open /Users/gameover/Desktop/StrangerDanger/flash/'twins 4 theory.app'");
			setState(kANAL_ERROR);
			LOG_ERROR("Something went terribly wrong with loading/analyzing the flash files...");
			abort();
		}
#else
		LOG_ERROR("GINGOLD! You gotta find an equivalent Windows command for system()!!!!!");
		abort();
#endif
		_processedFiles++;
		
		_flexComManager.setState(HAND);
		
		//setState(kANAL_CONNECTING);
		
	} 

}

//--------------------------------------------------------------
void Analyzer::serializeMessage(string & msg) {
	
	LOG_VERBOSE(msg);
	_publicMsg = msg;

	FileScenes * thisFileScene = _files[_processedFiles - 1];
	
	vector<string> chunks = ofSplitString(msg, ",");
	
	if (chunks.size() == 1) { // command message not serialize data
		
		vector<string> command = ofSplitString(chunks[0], ":");
		
		if (command[0] == "BYE" && _processedFiles == _files.size()) {
			LOG_NOTICE("Finished ANALyzing");
			setState(kANAL_FINISHED);
			command.clear();
		}
		
		if (command[0] == "GETCOMMAND") {
			
			
			
			if (thisFileScene->_processedScenes < thisFileScene->_mTypes.size()) {
				map<string, string>::iterator it;
				
				string fileName		= thisFileScene->_fileName;
				string sceneName	= thisFileScene->_scenes[thisFileScene->_processedScenes];
				
				it = thisFileScene->_mTypes.find(fileName + "_" + sceneName);
				
				string sendMSG = it->first + "," + it->second;
				
				LOG_VERBOSE("Sending command to flash with: " + sendMSG);
				
				_flexComManager.sendToAll("COMMAND:"+sendMSG);
				thisFileScene->_processedScenes++;
				
			} else {
				finalizeSerialization();
				_flexComManager.sendToAll("EXIT");
				command.clear();
				chunks.clear();
			}

			
		}
		
		if (command[0] == "SERIALIZE") {
			
			setState(kANAL_SERIALIZE);
			finalizeSerialization();

			_currentSceneName			= command[1];
			_currentSceneInteractivity	= command[2];
			
			command.clear();
			chunks.clear();
		}
		
	} // end chunks size == 1

	if (chunks.size() > 1) {
		
		LOG_NOTICE("Serializing data");
		
		int	frame;
		string scene, character;
		CamTransform data;		
		
		for (int i = 0; i < chunks.size(); i++) { // iterate the chunk
			
			vector<string> dataVec = ofSplitString(chunks[i], ":");
			
			string _type = dataVec[0];
			string _data = dataVec[1];
			
			//cout << _type << " = " << ofToFloat(_data) << endl;
			
			// scan _type and fill CamTransform class/struct
			if (_type == "x")  data.x		= ofToFloat(_data);
			if (_type == "y")  data.y		= ofToFloat(_data);
			if (_type == "w")  data.w		= ofToFloat(_data);
			if (_type == "h")  data.h		= ofToFloat(_data);
			if (_type == "sX") data.scaleX	= ofToFloat(_data);
			if (_type == "sY") data.scaleY	= ofToFloat(_data);
			if (_type == "r")  data.rotation = ofToFloat(_data);
			if (_type == "f")  frame			= ofToInt(_data);
			if (_type == "s")  scene			= _data;
			if (_type == "c")  character		= _data;
			
		} // end iterate chunk
		
		// mess with rotation values cos flash has a funny time with flipping transforms
		//if (data.scaleY < 0) {
			//data.rotation = -data.rotation;
		//}
		
		// set it to character name
		map< string, vector<CamTransform> >::iterator it;
		it = _currentTransformData.find(character);
		
		if (it == _currentTransformData.end()) { // not in map -> instantiate key/vector
			
			vector<CamTransform> vec;
			_currentTransformData[character] = vec;
			it = _currentTransformData.find(character); // set key to character name
			
		} // end not in map
		
		// push into the mapped vector<CamTransforms>
		if (frame > 1) it->second.push_back(data);
		
	}
}

void Analyzer::finalizeSerialization() {
	
	if (_currentTransformData.size() > 0) {	// save the last lot of transform data
		
		// setup path strings
		string scenePath = boost::any_cast<string>(_appModel->getProperty("scenesDataPath")) + "/" + ofSplitString(_currentSceneName, "_")[0];
		string fileName;
		string fullPath;
		
		// iterate each 'character'
		map< string, vector<CamTransform> >::iterator it;
		
		for (it = _currentTransformData.begin() ; it != _currentTransformData.end(); it++ ) {
			
			// save each characters' vector of transforms
			fileName = _currentSceneName + "_transform_" + (string)it->first + ".bin";
			fullPath = scenePath + "/" + fileName;

			saveVector(ofToDataPath(fullPath), &it->second);
			
			// print data just to be sure
			for (int i = 0; i < it->second.size(); i++) {
				
				stringstream out;
				
				out	<< it->first << " f: " << i 
				<< " x: " << it->second.at(i).x 
				<< " y: " << it->second.at(i).y
				<< " w: " << it->second.at(i).w
				<< " h: " << it->second.at(i).h
				<< " cX: " << (it->second.at(i).w/640.0f)
				<< " cY: " << (it->second.at(i).h/480.0f)
				<< " sX: " << it->second.at(i).scaleX 
				<< " sY: " << it->second.at(i).scaleY 
				<< " r: " << it->second.at(i).rotation;
				
				LOG_VERBOSE(out.str());
				
			} // end print
			
		} // end iterate each 'character'
		
		_currentSceneTotalFrames = _currentTransformData.size();
		_currentTransformData.clear();
		
		LOG_NOTICE("Attempting to parse interactivity map:" + _currentSceneInteractivity);
		
		vector<string> interactivityVec = ofSplitString(_currentSceneInteractivity, "|");
		
		SequenceDescriptor sd;
		
		sd._totalFrames = _currentSceneTotalFrames;
		
		sd._face = convertVecStringToFramePairs(interactivityVec[0]);
		sd._attacker = convertVecStringToFramePairs(interactivityVec[1]);
		sd._victim = convertVecStringToFramePairs(interactivityVec[2]);
		
		LOG_NOTICE("Attempting to save interactivity map");
		
		fileName = _currentSceneName + "_interactivity" + ".bin";
		fullPath = scenePath + "/" + fileName;
		
		saveClass(fullPath, &sd);
		
	} // end save the last lot of transform data
	
}

vector<FramePair> Analyzer::convertVecStringToFramePairs(string pairsAsString) {
	
	vector<FramePair> framePairs;
	
	vector<string> pairs = ofSplitString(pairsAsString, "#");
	
	for (int i = 0; i < pairs.size(); i++) {
		
		FramePair fp;
		
		vector<string> fpVec = ofSplitString(pairs[i], ";");
		
		fp._start	= ofToInt(fpVec[0]);
		fp._end		= ofToInt(fpVec[1]);
		
		if (fp._start == 0 && fp._end == -1) {
			fp._end = _currentSceneTotalFrames;
		}
		
		framePairs.push_back(fp);
		
	}
	
	return framePairs;
	
}

//--------------------------------------------------------------
string Analyzer::getMessage() {
	return printState() + " :: " + _publicMsg; 
}