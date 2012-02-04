/*
 *  SerializationUtils.h
 *  Jungle
 *
 *  Created by ollie on 12/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_SERIALIZATIONUTILS
#define	_H_SERIALIZATIONUTILS

#include <boost/serialization/serialization.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <string>
#include <vector>
using std::vector;


template <class vectorType>
bool loadVector(string filePath, vector< vectorType > * vec) {
	ofLog(OF_LOG_NOTICE, "Loading serial data: " + filePath);
	std::ifstream ifs(ofToDataPath(filePath).c_str());
	if(ifs.fail()){
		ofLog(OF_LOG_ERROR, "Could not load vector: " + filePath);
		abort(); // Could be a bit over zealous
	}
	boost::archive::text_iarchive ia(ifs);
	ia >> (*vec);
	return true;
}

template <class vectorType>
bool saveVector(string filePath, vector< vectorType > * vec) {
	ofLog(OF_LOG_NOTICE, "Saving serial data: " + filePath);
	std::ofstream ofs(ofToDataPath(filePath).c_str());
	if(ofs.fail()){
		ofLog(OF_LOG_ERROR, "Could not save vector: " + filePath);
		abort(); // Could be a bit over zealous
	}
    boost::archive::text_oarchive to(ofs);
    to << (*vec);
	return true;
}

template <class C>
bool loadClass(string filePath, C * someClass) {
	ofLog(OF_LOG_NOTICE, "loadClass: Loading class data: " + filePath);

	std::ifstream ifs(ofToDataPath(filePath).c_str());
	ofLog(OF_LOG_NOTICE, "loadClass: Created ifstream");
	if(ifs.fail()){
		ofLog(OF_LOG_ERROR, "loadClass: Could not load class: " + filePath);
		//abort(); // Could be a bit over zealous
		return false;
	}
	ofLog(OF_LOG_NOTICE, "loadClass: ifstream creation was successful");
	try {
	    ofLog(OF_LOG_NOTICE, "loadClass: Going to try creating text_iarchive from ifs ");
        boost::archive::text_iarchive ia(ifs);
        ofLog(OF_LOG_NOTICE, "loadClass: Successful creation of text_iarchive");
        ia >> (*someClass);
        ofLog(OF_LOG_NOTICE, "loadClass: Read text_iarchive into object, returning true");
        return true;
	} catch (boost::archive::archive_exception e) {
        ofLog(OF_LOG_ERROR, "loadClass: Error unserializing class from file: " + filePath);
        return false;
	}
	ofLog(OF_LOG_NOTICE, "loadClass: Should not get here, should return from try{}");
}

template <class C>
bool saveClass(string filePath, C * someClass) {
	ofLog(OF_LOG_NOTICE, "Saving class data: " + filePath);
	std::ofstream ofs(ofToDataPath(filePath).c_str());
	if(ofs.fail()){
		ofLog(OF_LOG_ERROR, "Could not class: " + filePath);
		//abort(); // Could be a bit over zealous
		return false;
	}
	try {
        boost::archive::text_oarchive to(ofs);
        to << (*someClass);
        return true;
	} catch (std::exception e) {
        ofLog(OF_LOG_ERROR, "Error serializing class from file: " + filePath);
        return false;
	}
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
