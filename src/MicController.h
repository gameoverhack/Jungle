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
};

// problems shutting down ofSoundStream, see: http://forum.openframeworks.cc/index.php?topic=5357.0 for solution

class MicController : public BaseState, public ofBaseApp {

public:

    MicController(string deviceName, int fftBufferLengthSecs, int audioBufferSize = 512, int sampleRate = 44100, int channels = 2);
    ~MicController();

	void	        registerStates();

    void            update();

    ofEvent<float>  victimAction;

    void            fakeVictimAction(float input);

private:

    void        audioReceived(float* input, int bufferSize, int nChannels);

    ofxFft *    _fft;

    int         _audioBufferSize;
    int         _fftCyclicBufferSize;
    int         _fftCyclicBufferOffset;
#if OF_VERSION > 6
    ofSoundStream soundStream;
#endif

};

#endif
