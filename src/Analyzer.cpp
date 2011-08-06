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
	
	//_sFiles.clear();
	_sScenes.clear();
	_files.clear();
	_scenes.clear();
	
	for (int i = 0; i < files->size(); i++) {
		
		vector<string> nameSplit = ofSplitString(files->at(i), ":");
		
		string fileName = nameSplit[0];
		string sceneName = nameSplit[1];
		string typeName = nameSplit[2];
		
		if (_sFiles.count(fileName) == 0) {
			_sFiles.insert(fileName);
			_files.push_back(fileName);
		}
		
		map<string, string>::iterator it;
		
		it = _sScenes.find(fileName + "_" + sceneName);
		if (it == _sScenes.end()) {
			_sScenes.insert(pair<string, string>(fileName + "_" + sceneName, typeName));
		} else {
			string allTypes = it->second + "-" + typeName;
			_sScenes[fileName + "_" + sceneName] = it->second + "-" + typeName;
		}
		
	}
	
	LOG_NOTICE("...actually checking " + ofToString((int)_scenes.size()) + " scenes in " + ofToString((int)_files.size()) + " files after duplicates extracted");
	
	_processedFiles = 0;
	
	//ofSetLogLevel(OF_LOG_VERBOSE);
	
	_flexComManager.setup(port);
	
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
		string filePath = "'" + flashPath + "/" + _files.at(_processedFiles) + ".app" + "'";
		
		systemMsg += filePath;
		
		LOG_NOTICE("Attempting to Load: " + filePath);
		
		int err = 0;//system(systemMsg.c_str());
		
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
		
		setState(kANAL_CONNECTING);
		
	} else if (_flexComManager.checkState(LOAD) && _processedFiles == _files.size()) {
		
		LOG_NOTICE("Finished ANALyzing");
		setState(kANAL_FINISHED);
	}

}

//--------------------------------------------------------------
void Analyzer::serializeMessage(string & msg) {
	
	LOG_VERBOSE(msg);
	_publicMsg = msg;
	//cout << msg << endl;
	
	vector<string> chunks = ofSplitString(msg, ",");
	
	if (chunks.size() == 1) { // command message not serialize data
		
		vector<string> command = ofSplitString(chunks[0], ":");
		
		//cout << command[1] << " =?= " << _sScenes.count(command[1]) << endl;
		
		bool doItBitch = true;
		if (command.size() > 0) {
			if (_sScenes.count(command[1]) == 0) {
				doItBitch = false;
			}
		}
		
		if (command[0] == "GETCOMMAND") {
			
			map<string, string>::iterator it;
			int index = 0;
			string sendMSG;
			
			for ( it = _sScenes.begin(); it != _sScenes.end(); it++) {
				cout << it->first + "," + it->second << endl;
			}
			
			cout << sendMSG << endl;
			
			_flexComManager.sendToAll("COMMAND:"+sendMSG);
		}
		
		if ((command[0] == "SERIALIZE" || command[0] == "EXIT") && doItBitch) {
			
			setState(kANAL_SERIALIZE);
			
			if (_currentTransformData.size() > 0 && _currentSceneName != "SKIP") {	// save the last lot of transform data
				
				// iterate each 'character'
				map< string, vector<CamTransform> >::iterator it;
				
				for (it = _currentTransformData.begin() ; it != _currentTransformData.end(); it++ ) {
					
					cout << "crash my crack" << endl;
					// save each characters' vector of transforms
					string scenePath = boost::any_cast<string>(_appModel->getProperty("scenesDataPath")) + "/" + "t";
					string fileName = _currentSceneName + "_transform_" + (string)it->first + ".bin";
					
					cout << "IMPLIMENT: " << scenePath << fileName << endl;
					saveVector(ofToDataPath(fileName), &it->second);
					
					// print data just to be sure
					for (int i = 0; i < it->second.size(); i++) {
						cout	<< it->first << " f: " << i 
						<< " x: " << it->second.at(i).x 
						<< " y: " << it->second.at(i).y
						<< " w: " << it->second.at(i).w
						<< " h: " << it->second.at(i).h
						<< " cX: " << (it->second.at(i).w/640.0f)
						<< " cY: " << (it->second.at(i).h/480.0f)
						<< " sX: " << it->second.at(i).scaleX 
						<< " sY: " << it->second.at(i).scaleY 
						<< " r: " << it->second.at(i).rotation << endl;
					} // end print
					
				} // end iterate each 'character'
				
			} // end save the last lot of transform data
			
			_currentTransformData.clear();
			
			if (command.size() > 0) _currentSceneName = command[1];
			
		} else if (command[0] != "EXIT" && command[0] != "BYE") {
			cout << "Stop being a fucking cunt" << endl;
			_currentSceneName = "SKIP";
			setState(kANAL_IGNORE);
			//_flexComManager.sendToAll("SKIP"); // too annoying to impliment correctly -> just ignore data for now
		}

		
	} // end chunks size == 1

	if (chunks.size() > 1 && _currentSceneName != "SKIP") {
		
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
			//CamTransform transform; // do it on frame ??
			//vec.assign(currentSceneTotalFrames, transform);
			_currentTransformData[character] = vec;
			it = _currentTransformData.find(character); // set key to character name
			
		} // end not in map
		
		// push into the mapped vector<CamTransforms>
		if (frame > 1) it->second.push_back(data);
		
	} else { // end chunk.size() > 1
		LOG_NOTICE("Skipping data");
	}
}

//--------------------------------------------------------------
string Analyzer::getMessage() {
	return printState() + " :: " + _publicMsg; 
}