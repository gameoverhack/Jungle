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

#include "goVideoPlayer.h"
#include "Logger.h"
#include "Constants.h"

class CamTransform {
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & x;
		ar & y;
		ar & w;
		ar & h;
		ar & scaleX;
		ar & scaleY;
		ar & rotation;
	}
public:
	float x;
	float y;
	float w;
	float h;
	float scaleX;
	float scaleY;
	float rotation;
	CamTransform(){};
	CamTransform(float _x, float _y, float _w, float _h, float _scaleX, float _scaleY, float _rotation) :
	x(_x), y(_y), w(_w), h(_h), scaleX(_scaleX), scaleY(_scaleY), rotation(_rotation)
	{}
};


class Sequence {

public:
	
	Sequence(){
		_movie = NULL;
		_isInteractive = false;
	};
	
	~Sequence(){
		delete _movie;
	};

	string getName(){
		return _name;
	}
	void setName(string name){
		_name = name;
	}
	
	void setNextSequenceName(string ns){
		_nextSequenceName = ns;
	}
	
	string getNextSequenceName(){
		return _nextSequenceName;
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
	
	void addTransform(vector<CamTransform> * trans){
		_transforms.push_back(trans);
	}	
	
	int	getTransformCount() {
		return _transforms.size();
	}
	
	vector<CamTransform> * getTransformVector(int i){
		if(i > _transforms.size() || i < 0){
			LOG_ERROR("Attempted to get transform for " + ofToString(i));
			abort();
		}
		return _transforms.at(i);
	}
	
	string getTransformAsString(int i, int f) {
		vector<CamTransform> * t = getTransformVector(i);
		string tranString = "f: " + ofToString(f) + 
							" x: " + ofToString(t->at(f).x) + 
							" y: " + ofToString(t->at(f).y) + 
							" w: " + ofToString(t->at(f).w) + 
							" h: " + ofToString(t->at(f).h) +
							" r: " + ofToString(t->at(f).rotation) + 
							" sX: " + ofToString(t->at(f).scaleX) + 
							" sY: " + ofToString(t->at(f).scaleY) +
							" zz: " + ofToString((int)t->size());
		return tranString;
	}
	
	void setSequenceMovie(goVideoPlayer *video){
		if(_movie != NULL){
			delete _movie;
		}
		_movie = video;
	}
	
	goVideoPlayer * getSequenceMovie(){
		return _movie;
	}
	
	void setIsInteractive(bool b){
		_isInteractive = b;
	}

	bool getIsInteractive(){
		return _isInteractive;
	}
	
	void setPaused(bool b) {
		_movie->setPaused(b);
	}
	
	void prepareSequenceMovie() {
		_movie->play();
		// must be set after play ?
		if(_isInteractive) {
			// loop on interactive movies
			_movie->setLoopState(OF_LOOP_NORMAL);
		} else {
			_movie->setLoopState(OF_LOOP_NONE);
		}
		setPaused(true);
	}
	
	void resetSequenceMovie() {
		if(_movie != NULL) {
			//_movie->stop();
			_movie->setPaused(true);
			_movie->setPosition(0.0);
		} else {
			LOG_ERROR("Could not reset sequence movie, movie == NULL");
			abort();
		}
	}
	
private:
	
	string					_name;
	string					_nextSequenceName;
	string					_attackerResult;
	string					_victimResult;
	bool					_isInteractive;

public:

	goVideoPlayer		* _movie;
	 
	vector< vector<CamTransform>* > _transforms; 
};


// Scene holds a map of Sequences, key'd by the sequence name

class Scene {
	
public:

	Scene(){
		_currentSequence = NULL;
	};

	~Scene(){
		map<string, Sequence *>::iterator iter;
		for(iter = _sequences.begin(); iter != _sequences.end(); iter++){
			delete (iter->second);
		}
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
			if(_currentSequence != NULL){
				// reset video state for current sequence
				_currentSequence->resetSequenceMovie();
			}

			// set new current sequence
			_currentSequence = iter->second;
			// play new current sequence movie
			_currentSequence->setPaused(false);
			
			LOG_NOTICE("Set current sequence to " + seq);
			return true;
		}
		else{
			LOG_ERROR("Unable to set current sequence to '" + seq + "' because its not in the map");
		}
		return false;
	}
	
	bool setCurrentSequence(Sequence * seq){
		// TODO:
		// Search through the map by value to find if the sequence is a valid
		// sequence? Linear search so "slow", (our data size is always going
		// to be so small it doesn't matter though.
		return setCurrentSequence(seq->getName());
	}
	
	Sequence * getCurrentSequence(){
		return _currentSequence;
	}
	
	// increments the current sequence to the next sequence (of the current sequence)
	bool nextSequence(){
		Sequence * seq = getCurrentSequence();
		/*
			we're on the last sequence of this scene if
				- sequence is not interactive
				- the next sequence for this sequence is its self
			Kind of magic logic, might be better to set either
				- bool _isLastSequence = true;
				or
				- string _nextSequence = "####FINALSEQUENCETOKEN####";

			Using the string means making a define
		 
		 */
		
		if(_currentSequence->getNextSequenceName()== kLAST_SEQUENCE_TOKEN){
			return false;
		}
		
		// not last sequence, so set next
		setCurrentSequence(_currentSequence->getNextSequenceName());
		return true;
	}
	
private:
	
	string					_name;
	
	Sequence *				_currentSequence;
	
	map<string, Sequence*>	_sequences;
	
	
};

#endif