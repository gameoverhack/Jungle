/*
 *  ArdController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "ArdController.h"

ArdController::ArdController(string deviceName, int ardBufferLengthSecs) {

    LOG_NOTICE("Constructing ArdController");

    registerStates();

    if(!_ard.connect(deviceName, 57600)) {

        LOG_ERROR("Cannot start Arduino on: " + deviceName);
        abort();

    } else {

        LOG_NOTICE("Successfully connected Arduino on: " + deviceName);

        // setup timeouts to simulate a similiar update lifecycle to the audio input
        _bufferIntervalMillis = 1000/60.0f;

        _ardCyclicBufferSize = ardBufferLengthSecs * _bufferIntervalMillis;
        _ardCyclicBufferOffset = 0;

        _appModel->setARDCyclicBufferSize(_ardCyclicBufferSize);
        _appModel->allocateARDCyclicBuffer(_ardCyclicBufferSize);
        _appModel->allocateARDNoiseFloor();
        _appModel->allocateARDCyclicSum();
        _appModel->allocateARDPostFilter();
        _appModel->allocatePinInput(2);

        setState(kARDCONTROLLER_INIT);
    }

}

ArdController::~ArdController() {
    LOG_NOTICE("Disconnecting Arduino");
    _ard.disconnect();
}

void ArdController::registerStates() {
	LOG_VERBOSE("Registering States");

    registerState(kARDCONTROLLER_RETARD, "kARDCONTROLLER_RETARD");
	registerState(kARDCONTROLLER_INIT, "kARDCONTROLLER_INIT");
	registerState(kARDCONTROLLER_READY, "kARDCONTROLLER_READY");
	registerState(kARDCONTROLLER_BELOWTHRESHOLD, "kARDCONTROLLER_BELOWTHRESHOLD");
	registerState(kARDCONTROLLER_ABOVETHRESHOLD, "kARDCONTROLLER_ABOVETHRESHOLD");

	setState(kARDCONTROLLER_RETARD);
}

void ArdController::update() {

    if (_ard.isArduinoReady()) {

		// 1st: setup the arduino if haven't already:
		if (checkState(kARDCONTROLLER_INIT)) {
			setupArduino();
		}
		// 2nd do the update of the arduino
		if (ofGetElapsedTimeMillis() - _lastUpdateTime > _bufferIntervalMillis) updateArduino();
	}

}

void ArdController::setupArduino() {

    LOG_VERBOSE("Setting up pin types on Arduino");

    _ard.sendAnalogPinReporting(0, ARD_ANALOG);	// AB: report data
	_ard.sendAnalogPinReporting(1, ARD_ANALOG);	// AB: report data

    ofSleepMillis(1000); // oh dear a magic number...does this stop the magic crashes??

    _lastUpdateTime = ofGetElapsedTimeMillis();

	setState(kARDCONTROLLER_READY);

}

void ArdController::updateArduino() {

    //LOG_VERBOSE("Update Ard");

    float * ardCyclicBuffer  = _appModel->getARDCyclicBuffer();
    float  ardNoiseFloor;//       = _appModel->getARDNoiseFloor();
    float  ardCyclicSum;//        = _appModel->getARDCyclicSum();
    float  ardPostFilter;//       = _appModel->getARDPostFilter();

    int * pinInput              = _appModel->getPinInput();

    _ard.update();

    pinInput[0] = _ard.getAnalog(0);
    pinInput[1] = _ard.getAnalog(1);

    _lastUpdateTime = ofGetElapsedTimeMillis();

    ardCyclicBuffer[_ardCyclicBufferOffset] = pinInput[0]/650.0f;

    ardCyclicSum = 0;

    for (int i = 0; i < _ardCyclicBufferSize; i++) {
        ardCyclicSum = ardCyclicSum + ardCyclicBuffer[i];
    }

    ardNoiseFloor = ardCyclicSum/(float)_ardCyclicBufferSize;
    ardPostFilter = pinInput[0]/650.0f - ardNoiseFloor;

    float area = sqrt(ardNoiseFloor) - 0.2f; //TODO: make prop

    _appModel->setARDArea(area);

    _ardCyclicBufferOffset = (_ardCyclicBufferOffset + 1) % _ardCyclicBufferSize;

    if (_appModel->checkState(kAPP_RUNNING)) {
        if (area > _appModel->getCurrentSequence()->getThresholdLevel()) {
            ofNotifyEvent(attackAction, area, this);
        }
    }

    //LOG_VERBOSE("[" + ofToString(ardRawPins[0]) + "::" + ofToString(ardRawPins[1]) + "]");

}
