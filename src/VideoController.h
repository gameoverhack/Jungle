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

	void	loadMovie(Sequence * seq, bool forceCurrentLoad = false, int lastFrameWhenForced = 0);
	void	toggleVideoPlayers(int lastFrameWhenForced = 0, bool noPause = false);

    void    reset();

	bool	isPreRolling();
    bool	_preRolling;

private:

	bool	_forceCurrentLoad;

	void	videoEvent(ofxThreadedVideoEvent & event);

	int     _lastFrameWhenForced;
	string  _lastSequenceWhenForced;

};

#endif
