/*
 *  ArdController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "ArdController.h"

ArdController::ArdController(string deviceName) {

    LOG_NOTICE("Constructing ArdController");

    registerStates();

    if(!_ard.connect(deviceName, 57600)) {
        _ard.setUseDelay(true);
        LOG_ERROR("Cannot start Arduino on: " + deviceName);
        abort();
    } else {
        LOG_NOTICE("Successfully connected Arduino on: " + deviceName);
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
		updateArduino();
	}

}

void ArdController::setupArduino() {

    LOG_VERBOSE("Setting up pin types on Arduino");

    _ard.sendAnalogPinReporting(0, ARD_ANALOG);	// AB: report data
	_ard.sendAnalogPinReporting(1, ARD_ANALOG);	// AB: report data

    ofSleepMillis(1000); // oh dear a magic number...does this stop the magic crashes??

	setState(kARDCONTROLLER_READY);

}

void ArdController::updateArduino() {

    //LOG_VERBOSE("Update Ard");

    _ard.update();

    int * pinInput = _appModel->getPinInput();

    pinInput[0] = _ard.getAnalog(0);
    pinInput[1] = _ard.getAnalog(1);

    //LOG_VERBOSE("[" + ofToString(ardRawPins[0]) + "::" + ofToString(ardRawPins[1]) + "]");

}
