/*
 *  VideoController.h
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_VIDEOCONTROLLER
#define _H_VIDEOCONTROLLER

#include "BaseState.h"
#include "AppModel.h"

enum {
	//kVIDCONTROLLER_UNREADY,
	kVIDCONTROLLER_READY,
	kVIDCONTROLLER_CURRENTVIDONE,
	kVIDCONTROLLER_NEXTVIDLOADING,
	kVIDCONTROLLER_NEXTVIDREADY,
	kVIDCONTROLLER_NEXTVIDERROR
};

class VideoController : public BaseState {

public:
	
	VideoController();
	~VideoController();
	
	void	registerStates();
	
	void	update();
	void	forceUpdate();
	
	void	loadMovie(Sequence * seq, bool forceCurrentLoad = false);
	void	toggleVideoPlayers();
	
private:
	
	bool	_cachedLoopAndState;
	bool	_forceCurrentLoad;
	
	void	loaded(string & path);
	void	error(int & err);
	
};

#endif