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
bool Logger::openLogFile(string filename)
{
    log(JU_LOG_NOTICE, typeid(this).name(), "Attempting to open log file...");
    _filename = filename;
    _toFile = true;
    _logFile.open(_filename.c_str(), ofstream::app);
    if (_logFile.good())
    {
        log(JU_LOG_NOTICE, typeid(this).name(), "//___________________________________________________\\");
        log(JU_LOG_NOTICE, typeid(this).name(), "Log started...");
        return true;
    }
    else
    {
        log(JU_LOG_NOTICE, typeid(this).name(), "Cannot open file");
        return false;
    }
}
//--------------------------------------------------------------
bool Logger::closeLogFile()
{
    log(JU_LOG_NOTICE, typeid(this).name(), "Closing log file");
    _toFile = false;
    _logFile.close();
    if (!_logFile.fail()) return true;
    else return false;
}
//--------------------------------------------------------------
void Logger::log(juLogLevel l, string objectName, string _msg)
{	
	if(l < _logLevel){
		return;
	}	
	stringstream outstring;
	outstring << "[" << gettimestamp() << "]: " << objectName << ":: " << _msg.c_str() << "\n";
    if(!_toFile)
    {
        cout << outstring.str();
    }
    else
    {
        cout << outstring.str();
        _logFile << outstring.str();
    }
}

void Logger::setLogLevel(juLogLevel l){
	_logLevel = l;
}

char * Logger::gettimestamp()
{
    sprintf(_timestamp, "%0.2d/%0.2i/%i - %0.2i:%0.2i:%0.2i", ofGetDay(), ofGetMonth(), ofGetYear(), ofGetHours(),  ofGetMinutes(), ofGetSeconds());
    return _timestamp;
}
//--------------------------------------------------------------
