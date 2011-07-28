/*
 *  BaseState.h
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_BASESTATE
#define _H_BASESTATE

#include "Logger.h"

#include <map>
#include <string>

enum {
	NONE = -1
};

class BaseState {
	
public:
	
	BaseState();
	virtual ~BaseState();
	
	virtual void	registerStates();
	
	int				getState();
	void			setState(int state);
	bool			checkState(int state);
	
	string			printState();
	
private:

	map<int, string>	_states;
	int					_state;
	
protected:
	
	void	registerState(int intState, string strState);
	
	//void	setState(string state); // do we need this?
	
};

#endif