/*
 *  BaseController.h
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _BASECONTROLLER_H
#define _BASECONTROLLER_H

#include "AppModel.h"
#include <map>
#include <string>

class BaseController {
	
public:
	
	BaseController();
	virtual ~BaseController();
	
	virtual void registerStates();
	
private:

	map<int, string>	_states;
	int					_state;
	
protected:
	
	void	registerState(int intState, string strState);
	
	int		getState();
	
	void	setState(int state);
	//void	setState(string state); // do we need this?
	
	void	printState();
	bool	checkState(int state);
	
};

#endif