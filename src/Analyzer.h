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
#include "SerializationUtils.h"

#include "goFlexCOM.h"

#ifdef TARGET_WIN32
#include <process.h>
#endif
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

class FileScenes{
public:
	string				_fileName;
	set<string>			_sScenes;
	vector<string>		_scenes;
	map<string, string>	_mTypes;
	int					_processedScenes;
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

	void				serializeMessage(string & msg);
	void				finalizeSerialization();
	vector<FramePair>	convertVecStringToFramePairs(string pairsAsString);

	string				_publicMsg;
	vector<FileScenes*>	_files;
	set<string>			_sFiles;
	string				_currentSceneName;
	string				_currentSceneInteractivity;
	int					_currentSceneTotalFrames;
	int					_processedFiles;
	goFlexCOM			_flexComManager;

	map< string, vector< CamTransform > >	_currentTransformData;

};

typedef Singleton<Analyzer> AnalyzerSingleton;

static Analyzer * _flashAnalyzer	= AnalyzerSingleton::Instance();

#endif
