/*
 *  Logger.cpp
 *  Jungle
 *
 *  Created by ollie on 1/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "Logger.h"

//--------------------------------------------------------------
bool Logger::openLogFile(string filename) {
	
    log(JU_LOG_NOTICE, typeid(this).name(), __func__, "Attempting to open log file...");
	
    _filename = filename;
    _toFile = true;
    _logFile.open(_filename.c_str(), ofstream::app);
	
    if (_logFile.good()) {
		
        log(JU_LOG_NOTICE, typeid(this).name(), __func__, "//___________________________________________________\\");
        log(JU_LOG_NOTICE, typeid(this).name(), __func__, "Log started...");
        return true;
		
    } else {
		
        log(JU_LOG_NOTICE, typeid(this).name(), __func__, "Cannot open file");
        return false;
		
    }
	
}

//--------------------------------------------------------------
bool Logger::closeLogFile() {
	
    log(JU_LOG_NOTICE, typeid(this).name(), __func__, "Closing log file");
	
    _toFile = false;
    _logFile.close();
	
    if (!_logFile.fail()) {
		return true;
	} else return false;
	
}

//--------------------------------------------------------------
void Logger::log(juLogLevel l, string objectName, string funcName, string msg) {
	
	if(l < _logLevel) return;

	stringstream outstring;
	string objectAndFunc = objectName+"::"+funcName;
	outstring << "[" << getTimeStamp() << " :: " << getLogLevelName(l) << " :: " << pad(objectAndFunc) << "]: " << msg.c_str() << "\n";
    
	if(_toFile) {
		
		cout << outstring.str();
        _logFile << outstring.str();
		
    } else cout << outstring.str();

}

inline string Logger::pad(string & t_string) {
	
	// possibly a more elegant sprintf solution for this but can't work out how to
	// dynamically set sprintf(objWithWhiteSpace, "%" + ofToString(_padLength) + "s", objectName) ???
	
	string paddedString = t_string;
	int padLength = 0;
	
	// check length and adjust overall pad if the objectName is longer than the current padLength
	if (t_string.size() > _padLength) _padLength = t_string.size();
	
	padLength = _padLength - t_string.size();
	
	for (int i = 0; i < padLength; i++) paddedString += " ";
	
	return paddedString;
}

inline string Logger::getLogLevelName(juLogLevel l) {
	
	switch(l){
		case JU_LOG_VERBOSE:
			return "VERB";
		case JU_LOG_NOTICE:
			return "NOTE";
		case JU_LOG_WARNING:
			return "WARN";
		case JU_LOG_ERROR:
			return "ERRR";
		case JU_LOG_FATAL_ERROR:
			return "FATE";
		default:
			return "UNKO";
	}
	
}

void Logger::setLogLevel(juLogLevel l){
	_logLevel = l;
}

inline char * Logger::getTimeStamp()
{
    sprintf(_timestamp, "%02i/%02i/%i - %02i:%02i:%02i", ofGetDay(), ofGetMonth(), ofGetYear(), ofGetHours(),  ofGetMinutes(), ofGetSeconds());
    return _timestamp;
}
//--------------------------------------------------------------
