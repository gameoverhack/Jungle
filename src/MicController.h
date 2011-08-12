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



class MicController : public BaseState, public ofBaseApp {

public:

    MicController(string deviceName, int fftBufferLengthSecs, int audioBufferSize = 512, int sampleRate = 44100, int channels = 2);
    ~MicController();

	void	        registerStates();

    void            update();

    ofEvent<float>  victimAction;

private:

    void        audioReceived(float* input, int bufferSize, int nChannels);

    ofxFft *    _fft;

    int         _audioBufferSize;
    int         _fftCyclicBufferSize;
    int         _fftCyclicBufferOffset;

};

#endif
