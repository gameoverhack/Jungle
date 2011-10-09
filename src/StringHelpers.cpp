/*
 *  StringHelpers.cpp
 *  Jungle
 *
 *  Created by ollie on 8/09/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "StringHelpers.h"

int findSequenceNumberFromString(string str){

	int seqNum;
	char seqNumString[20];
	string matchstring;
	cmatch match;
	
	// match seq01b0
	//seq(\d+)b(\d+)?
//	if(regex_match(str.c_str(), match, regex("seq(\\d+)b\\d+?"))){
//		matchstring = string(match[1].first, match[1].second);
//		sscanf(matchstring.c_str(), "%d", &seqNum); // extract int
//		//		LOG_NOTICE("Assigning number: " + ofToString(seqNum));
//		return seqNum;
//	}
	
	// pull out the sequence number from the sequence name, so we can increment it
	if(regex_match(str.c_str(), match, regex("seq(\\d+)\\D+?"))){ // find int
		matchstring = string(match[1].first, match[1].second);
		sscanf(matchstring.c_str(), "%d", &seqNum); // extract int
		//		LOG_NOTICE("Assigning number: " + ofToString(seqNum));
		return seqNum;
	}
	// cant log cause we're not in a class
	//	LOG_ERROR("Could not find sequence number from string: " + str);
	return -1;

}
