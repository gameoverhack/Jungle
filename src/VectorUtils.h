/*
 *  VectorUtils.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_VECTORUTILS
#define	_H_VECTORUTILS

#include <string>
#include <vector>
using std::vector;


template <class vectorType>
bool loadVector(string filePath, vector< vectorType > * vec) {
	ofLog(OF_LOG_VERBOSE, "Loading serial data: " + filePath);
	std::ifstream ifs(filePath.c_str());
	if(ifs.fail()){
		ofLog(OF_LOG_VERBOSE, "Could not load vector: " + filePath);
		abort(); // Could be a bit over zealous
	}
	boost::archive::text_iarchive ia(ifs);
	ia >> (*vec);
	return true;
}

template <class vectorType>
bool saveVector(string filePath, vector< vectorType > * vec) {
	ofLog(OF_LOG_VERBOSE, "Saving serial data: " + filePath);
	std::ofstream ofs(filePath.c_str());
	if(ofs.fail()){
		ofLog(OF_LOG_VERBOSE, "Could not save vector: " + filePath);
		abort(); // Could be a bit over zealous
	}
    boost::archive::text_oarchive to(ofs);
    to << (*vec);	
	return true;
}

// This causes duplicate symbols compiler errors
// no idea why, I think its something to do with the loadVector function??
//string concatenateVectorOfStrings(vector<string> strings, string seperator, bool addSpace = true){
//	string str = "";	
//	for(vector<string>::iterator iter = strings.begin(); iter != strings.end(); iter++){
//		str = *iter + seperator + (addSpace ? " " : "") + str;
//	}
//	
//	// remove last char
//	if(str.size () > 0){
//		str.resize (str.size () - 1);
//	}  
//	
//	return str;
//}


#endif