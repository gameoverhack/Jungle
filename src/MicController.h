/*
 *  MicController.h
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_MICCONTROLLER
#define _H_MICCONTROLLER

#include "BaseState.h"
#include "AppModel.h"

#include "ofxFft.h"

enum {
	kMICCONTROLLER_INIT,
	kMICCONTROLLER_READY,
	kMICCONTROLLER_BELOWTHRESHOLD,
	kMICCONTROLLER_ABOVETHRESHOLD
};



class MicController : public BaseState {

public:

    MicController();
    ~MicController();

	void	    registerStates();

    void        setup();
    void        update();

private:

    void        audioReceived(float* input, int bufferSize, int nChannels);

    ofxFft *    _fft;

};

#endif
