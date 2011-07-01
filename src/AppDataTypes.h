/*
 *  AppDataTypes.h
 *  Jungle
 *
 *  Created by gameover on 30/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */
#ifndef _H_APPDATATYPES
#define _H_APPDATATYPES

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "ofxAlphaVideoPlayer.h"

#define NUM_SCENES 1

class CamTransform {
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & x;
		ar & y;
		ar & scaleX;
		ar & scaleY;
		ar & rotation;
	}
public:
	float x;
	float y;
	float scaleX;
	float scaleY;
	float rotation;
	CamTransform(){};
	CamTransform(float _x, float _y, float _scaleX, float _scaleY, float _rotation) :
	x(_x), y(_y), scaleX(_scaleX), scaleY(_scaleY), rotation(_rotation)
	{}
};

/*
typedef struct {
	

	
} sequence;
*/

class Sequence {
public:
	
	Sequence(){};	// TODO: make constructor or is it too verbose?
	~Sequence(){};	// TODO: clean up?
			 
	string				 _name;
	string				 _attackerResult;
	string				 _victimResult;
	
	ofxAlphaVideoPlayer  _sequenceVideo;
	ofxAlphaVideoPlayer  _loopVideo;
	
	vector<CamTransform> _atk1Transforms;
	vector<CamTransform> _atk2Transforms;
	vector<CamTransform> _vicTransforms;
	
};

#endif