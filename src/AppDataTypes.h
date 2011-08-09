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

#include "goThreadedVideo.h"
#include "Logger.h"
#include "Constants.h"

//class VideoPlayer : public goThreadedVideo {
//	
//public:
//	
//	VideoPlayer(){};
//	~VideoPlayer(){};
//	
//	void	setCurrentFrame(int frame) {_currentFrame == frame;};
//	int		getCurrentFrame() {return _currentFrame;};
//		
//	
//private:
//
//	int					_currentFrame;
//	
//};


enum interaction_t {
	kINTERACTION_FACE,
	kINTERACTION_BOTH,
	kINTERACTION_VICTIM,
	kINTERACTION_ATTACKER,
	kINTERACTION_NONE
};

class FramePair{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & _start;
		ar & _end;
		
	}
	
public:
	int _start;
	int _end;
};

class SequenceDescriptor {
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & _totalFrames;
		ar & _face;
		ar & _attacker;
		ar & _victim;
	}
	
public:
	int _totalFrames;
	vector<FramePair> _face;
	vector<FramePair> _attacker;
	vector<FramePair> _victim;
};


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
	
	Sequence() {
		_isMovieFaked = false;
		_isSequenceFaked = false;
		_interactionTable = NULL;
	};
	
	~Sequence() {
		// nothing for now
		free(_interactionTable);
	};

	string getName() {
		return _name;
	}
	void setName(string name) {
		_name = name;
	}
	
	void setNextSequenceName(string ns) {
		_nextSequenceName = ns;
	}
	
	string getNextSequenceName() {
		return _nextSequenceName;
	}
	
	void setAttackerResult(string str) {
		_attackerResult = str;
	}
	string getAttackerResult() {
		return _attackerResult;
	}
	
	void setVictimResult(string str) {
		_victimResult = str;
	}
	string getVictimResult() {
		return _victimResult;
	}
	
	void setMovieFullFilePath(string path) {
		_movieFullFilePath = path;
	}
	
	string getMovieFullFilePath() {
		return _movieFullFilePath;
	}
	
	void setIsMovieFaked(bool b) {
		_isMovieFaked = b;
	}
	
	bool getIsMovieFaked() {
		return _isMovieFaked;
	}
	
	void setIsSequenceFaked(bool b){
		_isSequenceFaked = b;
	}
	
	bool getIsSequenceFaked(){
		return _isSequenceFaked;
	}
	
	void setTransform(string key, vector<CamTransform> * transform){
		_transforms.insert(make_pair(key, transform));
	}	
	
	int	getTransformCount() {
		return _transforms.size();
	}
	
	vector<CamTransform> * getTransformVector(string key) {
		map<string, vector<CamTransform> *>::iterator iter;
		iter = _transforms.find(key);
		if(iter == _transforms.end()){
			LOG_ERROR("Attempted to get transform for " + key);
			abort();
		}
		return iter->second;
	}
	
	string getTransformAsString(string key, int frame) {
		vector<CamTransform> * t = getTransformVector(key);
		string tranString = "f: " + ofToString(frame) + 
							" x: " + ofToString(t->at(frame).x) + 
							" y: " + ofToString(t->at(frame).y) + 
							" w: " + ofToString(t->at(frame).w) + 
							" h: " + ofToString(t->at(frame).h) +
							" r: " + ofToString(t->at(frame).rotation) + 
							" sX: " + ofToString(t->at(frame).scaleX) + 
							" sY: " + ofToString(t->at(frame).scaleY) +
							" zz: " + ofToString((int)t->size());
		return tranString;
	}
	
	void setInteractivity(string s) {
		_interactivity = s;
	}

	string getInteractivity() {
		return _interactivity;
	}
	
	string getType(){
		return _type;
	}
	
	void setType(string t){
		_type = t;
	}
	
	int getNumber(){
		return _number;
	}
	
	void setNumber(int i){
		_number = i;
	}
	
	interaction_t * getInteractionTable(){
		return _interactionTable;
	}
	
	void setInteractionTable(interaction_t *newInteractionTable){
		_interactionTable = newInteractionTable;
	}
	
	
private:
	
	string					_name;
	string					_type;
	int						_number;
	string					_nextSequenceName;
	string					_attackerResult;
	string					_victimResult;
	string					_interactivity;
	bool					_isMovieFaked;
	bool					_isSequenceFaked;
	string					_movieFullFilePath;
	
	interaction_t			*_interactionTable;

public:
	
	map<string, vector<CamTransform> *> _transforms;
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
	
	void rewindSequences() {
		_currentSequence = _sequences.begin()->second;
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
				//_currentSequence->resetMovie();
			}

			// set new current sequence
			_currentSequence = iter->second;
			// play new current sequence movie
			//_currentSequence->setPaused(false);
			
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
	
	bool hasCurrentSequence(){
		if(_currentSequence == NULL){
			return false;
		}
		return true;
	}
	
	// increments the current sequence to the next sequence (of the current sequence)
	bool nextSequence(){
		
		Sequence * seq = getCurrentSequence();
		LOG_VERBOSE(seq->getName());
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
			LOG_VERBOSE("It's not true");
			return false;
		}
		LOG_VERBOSE("It is true");
		// not last sequence, so set next
		setCurrentSequence(_currentSequence->getNextSequenceName());
		return true;
	}
	
	void print(){
		string tabs = "\t";
		string line = "Scene {";
		// print details of scene
		printf("%s\n", line.c_str());
		tabs = "\t\t";
		line = tabs + "_name => " + _name + "\n"+tabs+"_currentSequence => " + _currentSequence->getName();
		printf("%s\n", line.c_str());
		line = tabs + "sequences => {";
		printf("%s\n", line.c_str());
		tabs = "\t\t\t";
		map<string, Sequence*>::iterator iter;
		for(iter = _sequences.begin(); iter != _sequences.end(); iter++){
			line = tabs + "Sequence {";
			printf("%s\n", line.c_str());
			tabs = tabs + "\t";
			line = tabs + "name => " + iter->second->getName() + "\n";
			line = line + tabs + "nextSequence => " + iter->second->getNextSequenceName() + "\n";
			line = line + tabs + "victimResult => " + iter->second->getVictimResult() + "\n";
			line = line + tabs + "attackerResult => " + iter->second->getAttackerResult() + "\n";			
			printf("%s\n", line.c_str());
			tabs = "\t\t\t";
			line = tabs + "}";
			printf("%s\n", line.c_str());
		}
		tabs = "\t\t";
		line = tabs + "}";
		printf("%s\n", line.c_str());
		printf("}\n");
		
	}
	
private:
	
	string					_name;
	
	Sequence *				_currentSequence;
	
	map<string, Sequence*>	_sequences;
	
	
};

#endif