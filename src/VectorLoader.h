/*
 *  VectorLoader.h
 *  JungleApp
 *
 *  Created by ollie on 30/06/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_VECTORLOADER
#define _H_VECTORLOADER

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

template <class vectorType>
void loadVector(string filePath, vector< vectorType > & vec) {
	ofLog(OF_LOG_VERBOSE, ("LoadVector: " + filePath));
	std::ifstream ifs(filePath.c_str());
    boost::archive::text_iarchive ia(ifs);
    ia >> vec;	
}

#endif