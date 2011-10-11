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
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <map>
using std::map;
using std::pair;

#include "goThreadedVideo.h"
#include "goVideoGrabber.h"
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

// used for camera pos, rot scale
class PosRotScale {
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & x;
		ar & y;
        ar & r;
        ar & s;
	}

public:

    PosRotScale(){
        x = 0.0f;
        y = 0.0f;
        s = 0.5f;
        r = 0.0f;
    };

	float x;
	float y;
	float r;
	float s;

	string print(bool log) {
	    string out = "PRS: x = " + ofToString(x) + " y = " + ofToString(y) + " r = " + ofToString(r) + " s = " + ofToString(s);
	    if (log) cout << out << endl;
	    return out;
    }
};

// used for fft
typedef struct {
	float * fftBand;
} fftBands;

class FramePair{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & _start;
		ar & _end;
        ar & _to;
	}

public:
	int     _start;
	int     _end;
	string  _to;
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
		ar & _next;
	}

public:
	int _totalFrames;
	vector<FramePair> _face;
	vector<FramePair> _attacker;
	vector<FramePair> _victim;
	vector<FramePair> _next;
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

namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize(Archive & ar, setting & s, const unsigned int version) {
            ar & s.propName;
            ar & s.propID;
            ar & s.min;
            ar & s.max;
            ar & s.SteppingDelta;
            ar & s.CurrentValue;
            ar & s.DefaultValue;
            ar & s.flags;
            ar & s.pctValue;
        };
    };
};


class Settings {
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & settings;
	}
public:
    map<string, setting> settings;
	Settings(){};
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

	void setNextResult(string *str) { // technically doesn't need to be an array
		_nextResult = str;
	}

	string* getNextResult() { // technically doesn't need to be an array
		return _nextResult;
	}

	void setFaceResult(string *str) {
		_faceResult = str;
	}

	string* getFaceResult() {
		return _faceResult;
	}

	void setAttackerResult(string *str) {
		_attackerResult = str;
	}

	string* getAttackerResult() {
		return _attackerResult;
	}

	void setVictimResult(string *str) {
		_victimResult = str;
	}

	string* getVictimResult() {
		return _victimResult;
	}

	void setMovieFullFilePath(string path) {
		_movieFullFilePath = path;
	}

	string getMovieFullFilePath() {
		return _movieFullFilePath;
	}

    void setNumFrames(int frames) {_frames = frames;};
    int  getNumFrames() {return _frames;};

	void setIsMovieFaked(bool b) {
		_isMovieFaked = b;
	}

	bool getIsMovieFaked() {
		return _isMovieFaked;
	}

	void setIsSequenceFaked(bool b) {
		_isSequenceFaked = b;
	}

	bool getIsSequenceFaked() {
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

//	void setInteractivity(string s) {
//		_interactivity = s;
//	}
//
//	string getInteractivity() {
//		return _interactivity;
//	}

//	bool getLoop() {
//		return _loop;
//	}
//
//	void setLoop(bool loop){
//		_loop = loop;
//	}

	string getType() {
		return _type;
	}

	void setType(string t) {
		_type = t;
	}

	int getNumber() {
		return _number;
	}

	void setNumber(int i) {
		_number = i;
	}

	interaction_t * getInteractionTable() {
		return _interactionTable;
	}

	void setInteractionTable(interaction_t *newInteractionTable) {
		_interactionTable = newInteractionTable;
	}

    void setThresholdLevel(float level) {
        _thresholdLevel = level;
    }

    float getThresholdLevel() {
        return _thresholdLevel; // setting this in setNumber for now
    }

private:

	string					_name;
	string					_type;
	//bool                    _loop; // TODO: DEPRECIATED, use _type
	int						_number;
	int                     _frames;
	string*					_nextResult; // technically doesn't need to be an array
	string*                 _faceResult;
	string* 				_attackerResult;
	string*					_victimResult;
	//string					_interactivity; //TODO: DEPRECIATED
	bool					_isMovieFaked;
	bool					_isSequenceFaked;
	string					_movieFullFilePath;
    float                   _thresholdLevel;
	interaction_t*          _interactionTable;

public:

	// used for SceneXMLBuilder when it has to save out a model.
	vector<string> _transformsFilenames;
	string _interactivityFilename;

	map<string, vector<CamTransform> *> _transforms;
};


// Scene holds a map of Sequences, key'd by the sequence name

class Scene {

public:

	Scene(){
		_currentSequence = NULL;
	};

	~Scene() {
		map<string, Sequence *>::iterator iter;
		for(iter = _sequences.begin(); iter != _sequences.end(); iter++){
			delete (iter->second);
		}
	};

	int getNumOfSequences() {
		return _sequences.size();
	}

	map<string, Sequence*> getSequences(){
		return _sequences;
	}


	string getName() {
		return _name;
	}
	void setName(string name) {
		_name = name;
	}

	void rewindScene() {
		_currentSequence = _sequences.begin()->second; // this is an assumption...?
	}

	void setSequence(string key, Sequence * value) {
		_sequences.insert(pair<string, Sequence *>(key, value));
	}

    void setSequenceThresholds() {
        map<string, Sequence *>::iterator iter;
        int total   = _sequences.size(); // this is bad!
		for (iter = _sequences.begin(); iter != _sequences.end(); iter++) {
		    Sequence * seq = iter->second;
		    int num     = seq->getNumber();
		    float level = (5.0/(float)total)+((float)num/(float)total);
		    LOG_NOTICE("Set level threshold for: " + seq->getName() + " [" + ofToString(num) + " to " + ofToString(level));
            seq->setThresholdLevel(level);
		}
    }

	Sequence * getSequence(string key) {
		map<string, Sequence *>::iterator iter;
		iter = _sequences.find(key);
		return iter->second;
	}

    Sequence * getSequence(int num, string type) {
		map<string, Sequence *>::iterator iter;
		Sequence * seq = NULL;
		for (iter = _sequences.begin(); iter != _sequences.end(); iter++) {
		    seq = iter->second;
		    if (seq->getNumber() == num && seq->getType() == type) {
                break;
		    }
		}
		return seq;
	}

	bool setCurrentSequence(string seq) {
		map<string, Sequence *>::iterator iter;
		iter = _sequences.find(seq);
		if(iter != _sequences.end()){

			// set new current sequence
			_currentSequence = iter->second;

            // cache the last sequence name for returning after a b seq completes
            if (_currentSequence->getType() != "b") _lastSequenceName = _currentSequence->getName();

			LOG_NOTICE("Set current sequence to " + seq);
			return true;
		}
		else{
			LOG_ERROR("Unable to set current sequence to '" + seq + "' because its not in the map");
		}
		return false;
	}

	bool setCurrentSequence(Sequence * seq) {
		// TODO:
		// Search through the map by value to find if the sequence is a valid
		// sequence? Linear search so "slow", (our data size is always going
		// to be so small it doesn't matter though.

        // cache the last sequence name for returning after a b seq completes
        if (seq->getType() != "b") _lastSequenceName = seq->getName();
		return setCurrentSequence(seq->getName());
	}

	Sequence * getCurrentSequence() {
		return _currentSequence;
	}

	bool hasCurrentSequence(){
		if(_currentSequence == NULL) {
			return false;
		}
		return true;
	}

    void setTotalFrames(int frames) {_totalFrames = frames;};
    int  getTotalFrames() {return _totalFrames;};

	// increments the current sequence to the next sequence (of the current scene)
	bool nextSequence() {

        LOG_VERBOSE("Scene has been requested nextSequence...");

		Sequence * seq = getCurrentSequence();

		LOG_VERBOSE("..." + seq->getName());

        string nextResult = _currentSequence->getNextResult()[0];

		if(nextResult == "__FINAL_SEQUENCE__") { // all values in getNextResult (which is an array) should be set to correct value, so let's just use 0
		    LOG_VERBOSE("getNextSequenceName == __FINAL_SEQUENCE__");
            return false;
		}

		if (nextResult == "__RETURN_SEQUENCE__") { // so we can jump back to the correct sequence after a b movie is called
		    LOG_VERBOSE("getNextSequenceName == __RETURN_SEQUENCE__ => forcing to cached last sequence: " + _lastSequenceName);
            nextResult = _lastSequenceName;
		}

        //if (nextResult == "seq00a") nextResult = "seq01a"; // hacking face detection (this should only happen when AppController::update checks for VIDEOISDONE

		// not last sequence, so set next
		setCurrentSequence(nextResult); // all values in getNextResult (which is an array) should be set to correct value, so let's just use 0

		return true;
	}

	void print() {

		string tabs = "\t";
		string line = "Scene {";
		// print details of scene
		printf("%s\n", line.c_str());
		tabs = "\t\t";
		line = tabs + "name => " + _name + "\n";
		line = tabs + "totalFrames => " + ofToString(_totalFrames) + "\n";
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
			line = tabs + "duration => " + ofToString(iter->second->getNumFrames()) + "\n";
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

	Sequence*				_currentSequence;
    string                  _lastSequenceName;

	map<string, Sequence*>	_sequences;

	int                     _totalFrames; // not counting b or loop movies

};

#endif
