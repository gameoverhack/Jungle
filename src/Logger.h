/*
 *  Logger.h
 *  openFrameworks
 *
 *  Created by gameoverx on 31/08/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _H_LOGGER
#define _H_LOGGER

#include <iostream>
#include <fstream>
#include <typeinfo>

#include "Singleton.h"
#include "ofUtils.h"

enum juLogLevel{
	JU_LOG_VERBOSE,
	JU_LOG_NOTICE,
	JU_LOG_WARNING,
	JU_LOG_ERROR,
	JU_LOG_FATAL_ERROR
};

#define LOGGER			LoggerSingleton::Instance()

#define LOG(level, str)	LoggerSingleton::Instance()->log(level, typeid(*this).name(), (str))
#define LOG_ERROR(str)	LoggerSingleton::Instance()->log(JU_LOG_ERROR, typeid(*this).name(), (str))
#define LOG_WARNING(str) LoggerSingleton::Instance()->log(JU_LOG_WARNING, typeid(*this).name(), (str))
#define LOG_NOTICE(str)	LoggerSingleton::Instance()->log(JU_LOG_NOTICE, typeid(*this).name(), (str))

#define LOG_OPEN_FILE	LoggerSingleton::Instance()->openLogFile
//#define LOG_CLOSE_FILE	LoggerSingleton::Instance()->closeLogFile()

using namespace std;

class Logger
{
	
public:
	
    Logger()
    {
        _toFile = false;
		_logLevel = JU_LOG_WARNING;
		
		/* wont ever by written cause we default to a higher level of logging ... */
        log(JU_LOG_NOTICE, typeid(this).name(), "Created logger");
    };
	
    ~Logger()
    {	
        log(JU_LOG_NOTICE, typeid(this).name(), "Logging off...");
        log(JU_LOG_NOTICE, typeid(this).name(), "\\___________________________________________________//");	
        if (_toFile){
			closeLogFile();
		} 
    };
	
    bool	openLogFile(string _filename);
    bool	closeLogFile();
	
    void	log(juLogLevel l, string obj, string	_msg);
    char 	*gettimestamp();
	
	void	setLogLevel(juLogLevel l);
	
private:
	
    ofstream	_logFile;
    string		_filename;
	bool		_toFile;
	
    char		_timestamp[255];
    
	juLogLevel	_logLevel;

	
};

typedef Singleton<Logger> LoggerSingleton;   // Global declaration

#endif
