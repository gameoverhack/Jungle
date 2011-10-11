/*
 *  FunctionModel.h
 *  Bunyip
 *
 *  Created by gameover on 1/09/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_FUNCTIONMODEL
#define _H_FUNCTIONMODEL

#include "Singleton.h"
#include "Logger.h"

#include "FunctionDataTypes.h"

#include <map>
#include <vector>
#include <assert.h>
#include <string>

using std::map;
using std::vector;
using std::string;

class FunctionModel {
	
public:
	
	FunctionModel() {
		
		LOG_NOTICE("Constructing FunctionModel");
		
	};
	
	~FunctionModel() {
		
		LOG_NOTICE("Destroying FunctionModel");
		
	}

	// function registration for dynamic/virtualization
	template <class DelegateType>
	void registerFunction(string uniqueFunctionID, DelegateType delegate) {
		_registeredFunctions.insert(pair<string, DelegateMemento>(uniqueFunctionID, delegate.GetMemento()));
	};

	void unregisterFunction(string uniqueFunctionID) {
		map<string, DelegateMemento>::iterator it = _registeredFunctions.find(uniqueFunctionID);
		assert(it != _registeredFunctions.end());
		_registeredFunctions.erase(it);
	};
	
	map<string, DelegateMemento>	_registeredFunctions; // dangerous but copies and pointers don't seem to work
	
};

typedef Singleton<FunctionModel> FunctionModelSingleton;					// Global declaration

static FunctionModel * _functionModel	= FunctionModelSingleton::Instance();	// this is my idea of a compromise:
// no defines, keeps introspection
// but allows data sharing between classes


#endif