/*
 *  Analyzer.h
 *  Jungle
 *
 *  Created by gameover on 4/08/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_ANAL
#define _H_ANAL

//#include "Singleton.h"
#include "Logger.h"
#include "AppModel.h"
#include "BaseState.h"
#include "VectorUtils.h"

#include "goFlexCOM.h"

#include <vector>
#include <set>
#include <map>

using namespace std;

enum {
	kANAL_READY,
	kANAL_CONNECTING,
	kANAL_SERIALIZE,
	kANAL_IGNORE,
	kANAL_FINISHED,
	kANAL_ERROR
};

class Analyzer : public BaseState {

public:
	
	Analyzer();
	~Analyzer();
	
	void	registerStates();
	
	void	setup(vector<string> * files, int port = 6666);
	void	update();
	
	string	getMessage();
	
private:
	
	void serializeMessage(string & msg);
	
	string				_publicMsg;
	vector<string>		_files;
	vector<string>		_scenes;
	set<string>			_sFiles;
	map<string, string>	_sScenes;
	string				_currentSceneName;
	int					_processedFiles;
	goFlexCOM			_flexComManager;
	
	map< string, vector< CamTransform > >	_currentTransformData;
	
};

typedef Singleton<Analyzer> AnalyzerSingleton;

static Analyzer * _flashAnalyzer	= AnalyzerSingleton::Instance();

#endif