/*
 *  JungleExceptions.h
 *  Jungle
 *
 *  Created by ollie on 13/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_JUNGLEEXCEPTIONS
#define _H_JUNGLEEXCEPTIONS

#include <string>
#include <vector>
using namespace std;

// Jungle execption super class
class JungleException {

public:

	JungleException(string message) {
		_message = message;
	}

	string _message;
};

// Thrown when we can't parse the xml for whatever reason
// used to know when we should try a xml rebuild
class GenericXMLParseException : public JungleException {

public:

	GenericXMLParseException(string message) : JungleException(message) {
		// nothing?
	}
};

class XMLRebuildRequiredException : public JungleException{
public:
	XMLRebuildRequiredException(string message) : JungleException(message){
	}
};

class AnalysisRequiredException : public JungleException{
	vector<string> _files;
	
public:
	
	AnalysisRequiredException(string message, vector<string> files) : JungleException(message){
		_files = files;
		
	}
	
	vector<string> getFiles(){
		return _files;
	}
	
};


#endif
