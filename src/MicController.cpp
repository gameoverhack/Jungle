/*
 *  MicController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "MicController.h"

MicController::MicController(string deviceName, int fftBufferLengthSecs, int audioBufferSize, int sampleRate, int channels) {

    LOG_NOTICE("Constructing MicController");

    registerStates();

    // store audio buffer size (in samples) default = 512
    _audioBufferSize        = audioBufferSize;

    // calculate length of fftCyclicBuffer
    _fftCyclicBufferSize    = fftBufferLengthSecs * (sampleRate/_audioBufferSize);
    _fftCyclicBufferOffset  = 0;

        // create the fft
    LOG_NOTICE("Setting up fft");
#ifdef USE_FFTW
	_fft = ofxFft::create(_audioBufferSize, OF_FFT_WINDOW_HAMMING, OF_FFT_FFTW);
#else
	_fft = ofxFft::create(_audioBufferSize, OF_FFT_WINDOW_HAMMING);
#endif

     // allocate fft and audio sample arrays refs on the appModel...
    _appModel->setAudioBufferSize(_audioBufferSize);            // TODO: use this throughout instead of var
    _appModel->setFFTBinSize(_fft->getBinSize());               // TODO: use this throughout instead of var and instead of passing value
    _appModel->setFFTCyclicBufferSize(_fftCyclicBufferSize);    // TODO: use this throughout instead of var and instead of passing value
    _appModel->allocateFFTCyclicBuffer(_fftCyclicBufferSize, _fft->getBinSize());
    _appModel->allocateFFTNoiseFloor(_fft->getBinSize());
    _appModel->allocateFFTCyclicSum(_fft->getBinSize());
    _appModel->allocateFFTPostFilter(_fft->getBinSize());
    _appModel->allocateFFTInput(_fft->getBinSize());
    _appModel->allocateAudioInput(_audioBufferSize);

#ifdef TARGET_WIN32
    // instantiate the soundstream
    LOG_NOTICE("Setting up soundstream");
#if OF_VERSION < 7
    vector<ofStreamDevice> deviceVec = ofSoundStreamListDevices();

    int inputDeviceID = -1;
    for(int i = 0; i < deviceVec.size(); i++) {
        ofStreamDevice info = deviceVec[i];
        LOG_VERBOSE("Device name == " + info.name);
        if (info.name == deviceName) {
            inputDeviceID = i;
            break;
        }
    }
	ofSoundStreamSetup(0, channels, this, sampleRate, _audioBufferSize, 4, inputDeviceID);
#else
cout << "jesus" << endl;
    soundStream.setDeviceID(deviceName);
    cout << "jesuddds" << endl;
    ofSoundStreamSetup(0, channels, this, sampleRate, _audioBufferSize, 4);
#endif
#else
	LOG_WARNING("I made good changes to ofSoundStream -> impliment them by copying the file or making your own version");
	ofSoundStreamSetup(0, channels, this, sampleRate, _audioBufferSize, 4);
#endif

}

MicController::~MicController() {
    LOG_NOTICE("Closing soundstream...need to close fft too?????");
    delete _fft;
    _fft = NULL;
    ofSoundStreamClose(); // fucked up crash!!!
}

void MicController::registerStates() {
	LOG_VERBOSE("Registering States");

	registerState(kMICCONTROLLER_INIT, "kMICCONTROLLER_INIT");
	registerState(kMICCONTROLLER_READY, "kMICCONTROLLER_READY");

	setState(kMICCONTROLLER_INIT);
}

void MicController::update() {

    // update sound stream
    ofSoundUpdate();

}

void MicController::audioReceived(float* input, int bufferSize, int nChannels) {

    if (_fft != NULL) {

        // get fft and audio sample arrays refs from the appModel...
        fftBands * fftCyclicBuffer  = _appModel->getFFTCyclicBuffer();
        float * fftNoiseFloor       = _appModel->getFFTNoiseFloor();
        float * fftCyclicSum        = _appModel->getFFTCyclicSum();
        float * fftPostFilter       = _appModel->getFFTPostFilter();
        float * fftInput            = _appModel->getFFTInput();
        float * audioInput          = _appModel->getAudioInput();

        // put raw copy of audio input into model
        memcpy(audioInput, input, sizeof(float) * bufferSize);

        // pass raw audio data to the fft
        _fft->setSignal(audioInput);
        float * fftCurrent          = _fft->getAmplitude();

        // copy current fft into cyclic buffer
        for(int i = 0; i < _fft->getBinSize(); i++) {
            fftCyclicBuffer[_fftCyclicBufferOffset].fftBand[i] = fftInput[i] = fftCurrent[i];
        }

        // set cyclic sum back to 0;
        memset(fftCyclicSum, 0, sizeof(float) * _fft->getBinSize());

        // sum every band of every FFT in the cyclic buffer
        for (int i = 0; i < _fftCyclicBufferSize; i++) {
            for (int j = 0; j < _fft->getBinSize(); j++) {
                fftCyclicSum[j] = fftCyclicSum[j] + fftCyclicBuffer[i].fftBand[j];
            }
        }

        // calculate the Noise Floor (average of sum of all bands across the cyclic buffer, and
        // and simultaneously calculate the Adaptive Noise Reduced FFT, storing it on the model
        for (int j = 0; j < _fft->getBinSize(); j++) {
            fftNoiseFloor[j] = fftCyclicSum[j]/(float)_fftCyclicBufferSize;
            fftPostFilter[j] = fftInput[j] - fftNoiseFloor[j]*1.2f;
        }

        float area = 0;

        for (int i = 10; i < _fft->getBinSize(); i++) {
            float h = fftNoiseFloor[i]; //fftPostFilter[i]; // this is wrong but i like it!!
            float w = 1.0f;
            area += w*h;
        }

        area = sqrt(area) - 0.2f; //TODO: make prop

        _appModel->setFFTArea(area);

        // increase the cyclic buffer offset
        _fftCyclicBufferOffset = (_fftCyclicBufferOffset + 1) % _fftCyclicBufferSize;

        if (_appModel->checkState(kAPP_RUNNING)) {
            if (area > 1.0f) {
                ofNotifyEvent(victimAction, area, this);
                for (int i = 0; i < _fftCyclicBufferSize; i++) {
                    for (int j = 0; j < _fft->getBinSize(); j++) {
                        fftCyclicBuffer[i].fftBand[j] = fftCyclicSum[j] = fftNoiseFloor[j] = fftPostFilter[j] = 0;
                    }
                    _appModel->setFFTArea(0.0f);
                }
            }
        }
    }

}

void MicController::fakeVictimAction(float input) {

    //LOG_VERBOSE("Fake Victim Action " + ofToString(input));

    float * audioInput          = _appModel->getAudioInput();

    float * fakeInput = new float[_audioBufferSize];

   for (int i = 0; i < _audioBufferSize; i++) {
        fakeInput[i] = input;
   }

    audioReceived(fakeInput, _audioBufferSize, 2);

    delete [] fakeInput;

}
