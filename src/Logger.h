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

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

#define LOGGER				LoggerSingleton::Instance()

#define LOG(level, str)		LoggerSingleton::Instance()->log(level, typeid(*this).name(), __func__, (str))
#define LOG_ERROR(str)		LOG(JU_LOG_ERROR, (str))
#define LOG_WARNING(str)	LOG(JU_LOG_WARNING, (str))
#define LOG_NOTICE(str)		LOG(JU_LOG_NOTICE, (str))
#define LOG_VERBOSE(str)	LOG(JU_LOG_VERBOSE, (str))

#define LOG_OPEN_FILE	LoggerSingleton::Instance()->openLogFile
#define LOG_CLOSE_FILE	LoggerSingleton::Instance()->closeLogFile()

using namespace std;

class Logger {
	
public:
	
    Logger() {
		
        _toFile = false;
		_logLevel = JU_LOG_WARNING;
		_padLength	= 30; // default whitespace padding (self adjusting so we can set to 1 but could be any num)
		
		// wont ever by written cause we default to a higher level of logging ...
        log(JU_LOG_NOTICE, typeid(this).name(), __func__, "Created logger");
    
	};
	
    ~Logger() {	
		
        log(JU_LOG_NOTICE, typeid(this).name(), __func__, "Logging off...");
        log(JU_LOG_NOTICE, typeid(this).name(), __func__, "\\___________________________________________________//");	
        
		if (_toFile) closeLogFile();

    };
	
    bool	openLogFile(string _filename);
    bool	closeLogFile();
	
    void	log(juLogLevel l, string obj, string funcName, string msg);
	
	void	setLogLevel(juLogLevel l);
	
private:
	
	inline string	getLogLevelName(juLogLevel l);
	inline char 	*getTimeStamp();
	inline string	pad(string & objectName);
	
	int			_padLength;
	
    ofstream	_logFile;
    string		_filename;
	bool		_toFile;
	
    char		_timestamp[255];
    
	juLogLevel	_logLevel;

	
};

typedef Singleton<Logger> LoggerSingleton;   // Global declaration

#endif
