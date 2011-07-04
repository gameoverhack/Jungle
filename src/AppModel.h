/*
 *  AppModel.h
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_APPMODEL
#define	_H_APPMODEL

#include "ofMain.h"

#include "Singleton.h"
#include "AppDataTypes.h"

#include <boost/any.hpp>

#include <map>
#include <assert.h>

using std::map;

class AppModel {

public:
	
	AppModel();
	
	void setScene(string name, Scene * scene);
	bool setCurrentScene(string sceneName);

	Sequence * getCurrentSequence();
	Scene * getCurrentScene();
	
	bool nextScene();
	
	void setProperty(string propName, boost::any propVal);
	
	void getProperty(string propName, int & propVal);
	void getProperty(string propName, float & propVal);
	void getProperty(string propName, string & propVal);
	
	boost::any getProperty(string propName);

	string	getAllPropsAsList();
	
private:
	
	bool is_int(const boost::any & operand);
	bool is_float(const boost::any & operand);
	bool is_string(const boost::any & operand);
	bool is_char_ptr(const boost::any & operand);
	
	map<string, boost::any>				_anyProps;
//	map<string, Sequence*>				_scenes[NUM_SCENES];
	
	map<string, Scene *>				_scenes;
	
	Scene *							_currentScene;
	
protected:

};

typedef Singleton<AppModel> AppModelSingleton;					// Global declaration

static AppModel * _appModel	= AppModelSingleton::Instance();	// this is my idea of a compromise: 
																// no defines, keeps introspection
																// but allows data sharing between classes

#endif

