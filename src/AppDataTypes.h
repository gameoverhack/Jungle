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

#include <map>
using std::map;
using std::pair;

#include "ofxAlphaVideoPlayer.h"
#include "Logger.h"

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


class Sequence {

public:
	
	Sequence(){
		_movie = NULL;
	};
	
	~Sequence(){printf("Destructing sequence %s\n", _name.c_str());};	// TODO: clean up?

	string getName(){
		return _name;
	}
	void setName(string name){
		_name = name;
	}
	
	void setAttackerResult(string str){
		_attackerResult = str;
	}
	string getAttackerResult(){
		return _attackerResult;
	}
	
	void setVictimResult(string str){
		_victimResult = str;
	}
	string getVictimResult(){
		return _victimResult;
	}
	
	void addTransform(vector<CamTransform> & trans){
		_transforms.push_back(trans);
	}	
	
	vector<CamTransform> getTransformVector(int i){
		if(i >= 0 && i <= _transforms.size()){
			LOG_ERROR("Attempted to get transform for " + ofToString(i));
		}
		return _transforms.at(i);
	}
	
	void setSequenceMovie(ofxAlphaVideoPlayer *video){
		if(_movie != NULL){
			delete _movie;
		}
		_movie = video;
	}
	
	ofxAlphaVideoPlayer * getSequenceMovie(){
		return _movie;
	}
	
	void setIsInteractive(bool b){
		_isInteractive = b;
	}
		
private:
	
	string					_name;
	string					_attackerResult;
	string					_victimResult;
	bool					_isInteractive;

public:

	ofxAlphaVideoPlayer		* _movie;
	
	/*
		wanted to use pointers to avoid copying a vector each push_back
		but apparently you cant store pointers to templated vectors
		in a vector
	*/
	 
	vector< vector<CamTransform> > _transforms; 
};


/* 

 Scene holds a map of Sequences, key'd by the sequence name

*/
class Scene {
	
public:

	Scene(){};
	~Scene(){
		/* not sure if we have to delete the sequences here, 
		 I think we do because they are pointers not objects,
		 so the map isn't smart enough to call delete on them,
		 
		 can check this by putting a printf in ~Sequence()
		*/
	};
	
	int getNumOfSequences(){
		return _sequences.size();
	}
	
	string getName(){
		return _name;
	}
	void setName(string name){
		_name = name;
	}
	
	void setSequence(string key, Sequence * value){
		_sequences.insert(pair<string, Sequence *>(key, value));
	}
	
	Sequence * getSequence(string key){
		map<string, Sequence *>::iterator iter;
		iter = _sequences.find(key);
		return iter->second;
	}
	
	bool setCurrentSequence(string seq){
		map<string, Sequence *>::iterator iter;
		iter = _sequences.find(seq);
		if(iter != _sequences.end()){
			_currentSequence = iter->second; /* TODO: DO we have to dereference this? TEST IT. */
			LOG_NOTICE("Set current sequence to " + seq);
			return true;
		}
		else{
			LOG_ERROR("Unable to set current sequence to '" + seq + "' because its not in the map");
		}
		return false;
	}
	
	void setCurrentSequence(Sequence * seq){
		/* TODO:
			Search through the map by value to find if the sequence is a valid
			sequence? Linear search so "slow", (our data size is always going
			to be so small it doesn't matter though.
		*/
		_currentSequence = seq;
	}
	
	Sequence * getCurrentSequence(){
		return _currentSequence;
	}
	

private:
	
	string					_name;
	
	Sequence *				_currentSequence;
	
	map<string, Sequence*>	_sequences;
	
	
};

#endif