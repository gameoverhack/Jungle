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

// Jungle execption super class
class JungleException {

public:
	
	JungleException(string message){
		_message = message;
	}
	
	string _message;
};

// Thrown when we can't parse the xml for whatever reason
// used to know when we should try a xml rebuild
class GenericXMLParseException : public JungleException{

public:

	GenericXMLParseException(string message) : JungleException(message){
		// nothing?
	}
};

// Thrown when metadata (dates,sizes) don't match between the xml and hdd
class MetadataMismatchException : public JungleException{

public:
	
	MetadataMismatchException(string message) : JungleException(message){
		// nothing
	}
};


// Thrown when the number of frames don't match between the transforms and movies
class TransformMovieLengthMismatchException : public JungleException {

public:

	TransformMovieLengthMismatchException(string message, vector<string> names) : JungleException(message){
		_names = names;
	}

	vector<string> _names;

};

#endif