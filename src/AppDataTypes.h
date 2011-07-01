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


#include "VectorLoader.h"

class Sequence {

public:
	
	Sequence(string name){
		_name = name;
		LOG_NOTICE("Initialising with name: " + name);
		_victimResult = _name+"b";
		_attackerResult = _name+"a";
		_sequenceVideo.loadMovie("/Users/ollie/Source/of_62_osx/apps/stranger_danger_artifacts/t_seq_01_all_alpha_embedded2.mov");
		loadVector<CamTransform>(ofToDataPath("Tw Seq01_sg1_transform.bin"), _atk1Transforms);
		loadVector<CamTransform>(ofToDataPath("Tw Seq01_sg2_transform.bin"), _atk2Transforms);
		loadVector<CamTransform>(ofToDataPath("Tw Seq01_mm_transform.bin"), _vicTransforms);
		
	};	// TODO: make constructor or is it too verbose?
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
		
private:
	
	string					_name;
	string					_attackerResult;
	string					_victimResult;
	
public:
	ofxAlphaVideoPlayer		_sequenceVideo;
	ofxAlphaVideoPlayer		_loopVideo;

	//getTransformFor(player)
	
	map<string, CamTransform>	_transforms;
	
	vector<CamTransform>	_atk1Transforms;
	vector<CamTransform>	_atk2Transforms;
	vector<CamTransform>	_vicTransforms;
	
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
		return _numOfSequences;
	}
	void setNumOfSequences(int num){
		_numOfSequences = num;
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
	
	int						_numOfSequences;
	Sequence *				_currentSequence;
	
	
	map<string, Sequence*>	_sequences;
	
	
};

#endif