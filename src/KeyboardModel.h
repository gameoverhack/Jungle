/*
 *  KeyboardModel.h
 *  Bunyip
 *
 *  Created by gameover on 24/08/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_KEYBOARDMODEL
#define _H_KEYBOARDMODEL

#include "AppModel.h"

class KeyboardModel {
	
public:
	
	KeyboardModel() {
		LOG_NOTICE("Constructing KeyboardModel");
	};
	
	~KeyboardModel() {
		LOG_NOTICE("Destructing KeyboardModel");
	};	
	
	
	void registerEvent(int key, key_st keyType, string helpMessage, string functionToCall) {
		KeyMessage keyboardMessage(key, keyType, 0);
		InputParams0<KeyMessage> inputParams(keyboardMessage);
		registerInputToFunction(functionToCall, inputParams, helpMessage);
	}
	
	template<class Param1>
	void registerEvent(int key, key_st keyType, string helpMessage, string functionToCall, Param1 param1) {
		KeyMessage keyboardMessage(key, keyType, 0);
		InputParams1<KeyMessage, Param1> inputParams(keyboardMessage, param1);
		registerInputToFunction(functionToCall, inputParams, helpMessage);
	}
	
	template<class Param1>
	void registerEvent(int key, key_st keyType, string helpMessage, string functionToCall, Param1 &param1, bool variableIsPointer) {
		KeyMessage keyboardMessage(key, keyType, 0);
		InputParams1<KeyMessage, Param1> inputParams(keyboardMessage, &param1);
		registerInputToFunction(functionToCall, inputParams, helpMessage);
	}
	
	template<class Param1, class Param2>
	void registerEvent(int key, key_st keyType, string helpMessage, string functionToCall, Param1 param1, Param2 param2) {
		KeyMessage keyboardMessage(key, keyType, 0);
		InputParams2<KeyMessage, Param1, Param2> inputParams(keyboardMessage, param1, param2);
		registerInputToFunction(functionToCall, inputParams, helpMessage);
	}
	
	template<class Param1, class Param2>
	void registerEvent(int key, key_st keyType, string helpMessage, string functionToCall, Param1 &param1, Param2 param2, bool parameter1IsPointer) {
		KeyMessage keyboardMessage(key, keyType, 0);
		InputParams2<KeyMessage, Param1, Param2> inputParams(keyboardMessage, &param1, param2);
		registerInputToFunction(functionToCall, inputParams, helpMessage);
	}
	
	template<class Param1, class Param2>
	void registerEvent(int key, key_st keyType, string helpMessage, string functionToCall, Param1 &param1, Param2 &param2, bool parameter1IsPointer, bool parameter2IsPointer) {
		KeyMessage keyboardMessage(key, keyType, 0);
		InputParams2<KeyMessage, Param1, Param2> inputParams(keyboardMessage, &param1, &param2);
		registerInputToFunction(functionToCall, inputParams, helpMessage);
	}
	
	template <class InputParams>
	void registerInputToFunction(string uniqueFunctionID, InputParams inputParameters, string usage = "") {
		
		map<string, DelegateMemento>::iterator it = _functionModel->_registeredFunctions.find(uniqueFunctionID);
		
		if (it != _functionModel->_registeredFunctions.end()) {
			
			LOG_VERBOSE("Adding function map for: " + uniqueFunctionID);
			MappedDelegate<InputParams> md(uniqueFunctionID, it->second, inputParameters);
			setMappedDelegate(inputParameters.getMessageMap(), md);
			
			if (usage == "") {
				int nParams = inputParameters.getNumParams();
				ostringstream params;
				switch (nParams) {
					case 0:
						params << "()";
						break;
					case 1:
						params << "(" << inputParameters.getParam1() << ")";
						break;
					case 2:
						params << "(" << inputParameters.getParam1() << ", " << inputParameters.getParam1() << ")";
						break;
				}
				
				usage = uniqueFunctionID + params.str();
			}
			
			storeUsage(inputParameters.getMessageMap(), usage);
			
		} else {
			
			LOG_VERBOSE("No function registered for: " + uniqueFunctionID);
			abort();
			
		}
		
	}
	
	string getAllKeyboardUsageAsList(bool log = false) {
		
		ostringstream out;
		for (int i = 0; i < _keyboardUsage.size(); i++) {
			out << _keyboardUsage[i] << endl;
		}
		
		if (log) {
			LOG_NOTICE(out.str());
		}
		
		return out.str();
		
	};
	
	void executeFunction(int key, key_st status) {
		
		KeyMessage keyboardMessage(key, status, 0);
		//keyboardMessage.print(true);
		
		map<KeyMessage, DelegateType>::iterator it_type;
		
		it_type = _mappedKeyboardType.find(keyboardMessage);
		
		if (it_type != _mappedKeyboardType.end()) {
			
			DelegateType dt = it_type->second;
			
			switch (dt) {
				case GO_DELEGATE_NONE:
				{
					map<KeyMessage, MappedDelegate<InputParams0<KeyMessage>			> >	 ::iterator	it0;
					it0  = _mappedKeyboardParams0.find(keyboardMessage);
					DelegateMemento dm = it0->second.getDelegateMemento();
					Delegate0 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec();
					break;
				}
				case GO_DELEGATE_STRING:
				{
					map<KeyMessage, MappedDelegate<InputParams1<KeyMessage, string	> > >::iterator	it1s;
					it1s = _mappedKeyboardParamsString1.find(keyboardMessage);
					DelegateMemento dm = it1s->second.getDelegateMemento();
					DelegateString1 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it1s->second.getInputParams().getParam1());
					break;
				}
				case GO_DELEGATE_INT:
				{
					map<KeyMessage, MappedDelegate<InputParams1<KeyMessage, int		> > >::iterator	it1i;
					it1i = _mappedKeyboardParamsInt1.find(keyboardMessage);
					DelegateMemento dm = it1i->second.getDelegateMemento();
					DelegateInt1 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it1i->second.getInputParams().getParam1());
					break;
				}
				case GO_DELEGATE_FLOAT:
				{
					map<KeyMessage, MappedDelegate<InputParams1<KeyMessage, float	> > >::iterator	it1f;
					it1f = _mappedKeyboardParamsFloat1.find(keyboardMessage);
					DelegateMemento dm = it1f->second.getDelegateMemento();
					DelegateFloat1 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it1f->second.getInputParams().getParam1());
					break;
				}
				case GO_DELEGATE_BOOL:
				{
					map<KeyMessage, MappedDelegate<InputParams1<KeyMessage, bool	> > >::iterator	it1b;
					it1b = _mappedKeyboardParamsBool1.find(keyboardMessage);
					DelegateMemento dm = it1b->second.getDelegateMemento();
					DelegateBool1 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it1b->second.getInputParams().getParam1());
					break;
				}
				case GO_DELEGATE_STRING_STRING:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, string > > >::iterator	it2ss;
					it2ss = _mappedKeyboardParamsStringString2.find(keyboardMessage);
					DelegateMemento dm = it2ss->second.getDelegateMemento();
					DelegateStringString2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2ss->second.getInputParams().getParam1(), it2ss->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_INT_INT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, int > > >::iterator	it2ii;
					it2ii = _mappedKeyboardParamsIntInt2.find(keyboardMessage);
					DelegateMemento dm = it2ii->second.getDelegateMemento();
					DelegateIntInt2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2ii->second.getInputParams().getParam1(), it2ii->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_FLOAT_FLOAT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, float > > >::iterator	it2ff;
					it2ff = _mappedKeyboardParamsFloatFloat2.find(keyboardMessage);
					DelegateMemento dm = it2ff->second.getDelegateMemento();
					DelegateFloatFloat2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2ff->second.getInputParams().getParam1(), it2ff->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_BOOL_BOOL:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, bool > > >::iterator	it2bb;
					it2bb = _mappedKeyboardParamsBoolBool2.find(keyboardMessage);
					DelegateMemento dm = it2bb->second.getDelegateMemento();
					DelegateBoolBool2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2bb->second.getInputParams().getParam1(), it2bb->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_STRING_INT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, int > > >::iterator	it2si;
					it2si = _mappedKeyboardParamsStringInt2.find(keyboardMessage);
					DelegateMemento dm = it2si->second.getDelegateMemento();
					DelegateStringInt2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2si->second.getInputParams().getParam1(), it2si->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_STRING_FLOAT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, float > > >::iterator	it2sf;
					it2sf = _mappedKeyboardParamsStringFloat2.find(keyboardMessage);
					DelegateMemento dm = it2sf->second.getDelegateMemento();
					DelegateStringFloat2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2sf->second.getInputParams().getParam1(), it2sf->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_STRING_BOOL:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, bool > > >::iterator	it2sb;
					it2sb = _mappedKeyboardParamsStringBool2.find(keyboardMessage);
					DelegateMemento dm = it2sb->second.getDelegateMemento();
					DelegateStringBool2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2sb->second.getInputParams().getParam1(), it2sb->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_INT_STRING:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, string > > >::iterator	it2is;
					it2is = _mappedKeyboardParamsIntString2.find(keyboardMessage);
					DelegateMemento dm = it2is->second.getDelegateMemento();
					DelegateIntString2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2is->second.getInputParams().getParam1(), it2is->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_INT_FLOAT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, float > > >::iterator	it2if;
					it2if = _mappedKeyboardParamsIntFloat2.find(keyboardMessage);
					DelegateMemento dm = it2if->second.getDelegateMemento();
					DelegateIntFloat2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2if->second.getInputParams().getParam1(), it2if->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_INT_BOOL:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, bool > > >::iterator	it2ib;
					it2ib = _mappedKeyboardParamsIntBool2.find(keyboardMessage);
					DelegateMemento dm = it2ib->second.getDelegateMemento();
					DelegateIntBool2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2ib->second.getInputParams().getParam1(), it2ib->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_FLOAT_STRING:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, string > > >::iterator	it2fs;
					it2fs = _mappedKeyboardParamsFloatString2.find(keyboardMessage);
					DelegateMemento dm = it2fs->second.getDelegateMemento();
					DelegateFloatString2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2fs->second.getInputParams().getParam1(), it2fs->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_FLOAT_INT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, int > > >::iterator	it2fi;
					it2fi = _mappedKeyboardParamsFloatInt2.find(keyboardMessage);
					DelegateMemento dm = it2fi->second.getDelegateMemento();
					DelegateFloatInt2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2fi->second.getInputParams().getParam1(), it2fi->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_FLOAT_BOOL:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, bool > > >::iterator	it2fb;
					it2fb = _mappedKeyboardParamsFloatBool2.find(keyboardMessage);
					DelegateMemento dm = it2fb->second.getDelegateMemento();
					DelegateFloatBool2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2fb->second.getInputParams().getParam1(), it2fb->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_BOOL_STRING:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, string > > >::iterator	it2bs;
					it2bs = _mappedKeyboardParamsBoolString2.find(keyboardMessage);
					DelegateMemento dm = it2bs->second.getDelegateMemento();
					DelegateBoolString2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2bs->second.getInputParams().getParam1(), it2bs->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_BOOL_INT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, int > > >::iterator	it2bi;
					it2bi = _mappedKeyboardParamsBoolInt2.find(keyboardMessage);
					DelegateMemento dm = it2bi->second.getDelegateMemento();
					DelegateBoolInt2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2bi->second.getInputParams().getParam1(), it2bi->second.getInputParams().getParam2());
					break;
				}
				case GO_DELEGATE_BOOL_FLOAT:
				{
					map<KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, float > > >::iterator	it2bf;
					it2bf = _mappedKeyboardParamsBoolFloat2.find(keyboardMessage);
					DelegateMemento dm = it2bf->second.getDelegateMemento();
					DelegateBoolFloat2 funcToExec;
					funcToExec.SetMemento(dm);
					funcToExec(it2bf->second.getInputParams().getParam1(), it2bf->second.getInputParams().getParam2());
					break;
				}
			}
		}
		
	}
	
private:
	
	void storeUsage(KeyMessage keyboardMessage, string usage) {
		
		char c = (char)keyboardMessage.getKey();
		ostringstream usestream;
		
		usestream << "'" << c << "'" << " - " << usage;
		
		usage = usestream.str();
		
		LOG_VERBOSE("Created key usage: " + usage);
		
		_keyboardUsage.push_back(usage);
	}
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams0<KeyMessage> > md) {
		LOG_VERBOSE("Store p0");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_NONE));
		_mappedKeyboardParams0.insert(pair<KeyMessage, MappedDelegate< InputParams0<KeyMessage> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams1<KeyMessage, string> > md) {
		LOG_VERBOSE("Store p1 string");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_STRING));
		_mappedKeyboardParamsString1.insert(pair< KeyMessage, MappedDelegate< InputParams1< KeyMessage, string> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams1<KeyMessage, float> > md) {
		LOG_VERBOSE("Store p1 float");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_FLOAT));
		_mappedKeyboardParamsFloat1.insert(pair< KeyMessage, MappedDelegate<InputParams1<KeyMessage, float> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams1<KeyMessage, int> > md) {
		LOG_VERBOSE("Store p1 int");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_INT));
		_mappedKeyboardParamsInt1.insert(pair< KeyMessage, MappedDelegate< InputParams1<KeyMessage, int> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams1<KeyMessage, bool> > md) {
		LOG_VERBOSE("Store p1 bool");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_BOOL));
		_mappedKeyboardParamsBool1.insert(pair< KeyMessage, MappedDelegate<InputParams1<KeyMessage, bool> >	> (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, string, string> > md) {
		LOG_VERBOSE("Store p2 string string");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_STRING_STRING));
		_mappedKeyboardParamsStringString2.insert(pair< KeyMessage, MappedDelegate< InputParams2< KeyMessage, string, string> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, string, float> > md) {
		LOG_VERBOSE("Store p2 string float");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_STRING_FLOAT));
		_mappedKeyboardParamsStringFloat2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, float> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, string, int> > md) {
		LOG_VERBOSE("Store p2 string int");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_STRING_INT));
		_mappedKeyboardParamsStringInt2.insert(pair< KeyMessage, MappedDelegate< InputParams2<KeyMessage, string, int> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, string, bool> > md) {
		LOG_VERBOSE("Store p2 string bool");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_STRING_BOOL));
		_mappedKeyboardParamsStringBool2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, bool> >	> (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, float, float> > md) {
		LOG_VERBOSE("Store p2 float float");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_FLOAT_FLOAT));
		_mappedKeyboardParamsFloatFloat2.insert(pair< KeyMessage, MappedDelegate< InputParams2< KeyMessage, float, float> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, float, string> > md) {
		LOG_VERBOSE("Store p2 float string");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_FLOAT_STRING));
		_mappedKeyboardParamsFloatString2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, string> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, float, int> > md) {
		LOG_VERBOSE("Store p2 float int");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_FLOAT_INT));
		_mappedKeyboardParamsFloatInt2.insert(pair< KeyMessage, MappedDelegate< InputParams2<KeyMessage, float, int> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, float, bool> > md) {
		LOG_VERBOSE("Store p2 float bool");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_FLOAT_BOOL));
		_mappedKeyboardParamsFloatBool2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, bool> >	> (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, int, int> > md) {
		LOG_VERBOSE("Store p2 int int");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_INT_INT));
		_mappedKeyboardParamsIntInt2.insert(pair< KeyMessage, MappedDelegate< InputParams2< KeyMessage, int, int> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, int, string> > md) {
		LOG_VERBOSE("Store p2 int string");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_INT_STRING));
		_mappedKeyboardParamsIntString2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, string> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, int, float> > md) {
		LOG_VERBOSE("Store p2 int float");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_INT_FLOAT));
		_mappedKeyboardParamsIntFloat2.insert(pair< KeyMessage, MappedDelegate< InputParams2<KeyMessage, int, float> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, int, bool> > md) {
		LOG_VERBOSE("Store p2 int bool");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_INT_BOOL));
		_mappedKeyboardParamsIntBool2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, bool> >	> (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, bool, bool> > md) {
		LOG_VERBOSE("Store p2 bool bool");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_BOOL_BOOL));
		_mappedKeyboardParamsBoolBool2.insert(pair< KeyMessage, MappedDelegate< InputParams2< KeyMessage, bool, bool> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, bool, string> > md) {
		LOG_VERBOSE("Store p2 bool string");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_BOOL_STRING));
		_mappedKeyboardParamsBoolString2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, string> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, bool, float> > md) {
		LOG_VERBOSE("Store p2 bool float");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_BOOL_FLOAT));
		_mappedKeyboardParamsBoolFloat2.insert(pair< KeyMessage, MappedDelegate< InputParams2<KeyMessage, bool, float> > > (keyboardMessage, md));
	};
	
	void setMappedDelegate(KeyMessage keyboardMessage, MappedDelegate<InputParams2<KeyMessage, bool, int> > md) {
		LOG_VERBOSE("Store p2 bool int");
		_mappedKeyboardType.insert(pair<KeyMessage, DelegateType> (keyboardMessage, GO_DELEGATE_BOOL_INT));
		_mappedKeyboardParamsBoolInt2.insert(pair< KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, int> >	> (keyboardMessage, md));
	};
	
	// making these public for now until i work out a nicer way to return
	map< KeyMessage, DelegateType >													_mappedKeyboardType;
	vector<string>																	_keyboardUsage;
	
	map< KeyMessage, MappedDelegate<InputParams0<KeyMessage> > >					_mappedKeyboardParams0;
	
	map< KeyMessage, MappedDelegate<InputParams1<KeyMessage, string> > >			_mappedKeyboardParamsString1;
	map< KeyMessage, MappedDelegate<InputParams1<KeyMessage, float> > >				_mappedKeyboardParamsFloat1;
	map< KeyMessage, MappedDelegate<InputParams1<KeyMessage, int> > >				_mappedKeyboardParamsInt1;
	map< KeyMessage, MappedDelegate<InputParams1<KeyMessage, bool> > >				_mappedKeyboardParamsBool1;
	
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, string> > >	_mappedKeyboardParamsStringString2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, float> > >		_mappedKeyboardParamsStringFloat2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, int> > >		_mappedKeyboardParamsStringInt2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, string, bool> > >		_mappedKeyboardParamsStringBool2;
	
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, float> > >		_mappedKeyboardParamsFloatFloat2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, string> > >		_mappedKeyboardParamsFloatString2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, int> > >		_mappedKeyboardParamsFloatInt2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, float, bool> > >		_mappedKeyboardParamsFloatBool2;
	
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, int> > >			_mappedKeyboardParamsIntInt2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, string> > >		_mappedKeyboardParamsIntString2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, float> > >		_mappedKeyboardParamsIntFloat2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, int, bool> > >			_mappedKeyboardParamsIntBool2;
	
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, bool> > >		_mappedKeyboardParamsBoolBool2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, string> > >		_mappedKeyboardParamsBoolString2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, float> > >		_mappedKeyboardParamsBoolFloat2;
	map< KeyMessage, MappedDelegate<InputParams2<KeyMessage, bool, int> > >			_mappedKeyboardParamsBoolInt2;
	
};

typedef Singleton<KeyboardModel> KeyboardModelSingleton;					// Global declaration

static KeyboardModel * _keyModel	= KeyboardModelSingleton::Instance();	// this is my idea of a compromise:
// no defines, keeps introspection
// but allows data sharing between classes

#endif