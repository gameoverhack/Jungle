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

#endif