/*
 *  ArdController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "ArdController.h"

ArdController::ArdController() {
    // nothing?
}

ArdController::~ArdController() {
    LOG_NOTICE("Disconnecting Arduino");
    _ard.disconnect();
}

void ArdController::registerStates() {
	LOG_VERBOSE("Registering States");

	registerState(kARDCONTROLLER_INIT, "kARDCONTROLLER_INIT");
	registerState(kARDCONTROLLER_READY, "kARDCONTROLLER_READY");
	registerState(kARDCONTROLLER_BELOWTHRESHOLD, "kARDCONTROLLER_BELOWTHRESHOLD");
	registerState(kARDCONTROLLER_ABOVETHRESHOLD, "kARDCONTROLLER_ABOVETHRESHOLD");

	setState(kARDCONTROLLER_INIT);
}

void ArdController::setup(string deviceName) {
    if(!_ard.connect(deviceName, 57600) && _ard.isInitialized()) {
        //_ard.setUseDelay(true);
        LOG_ERROR("Cannot start Arduino on: " + deviceName);
        abort();
    } else LOG_NOTICE("Successfully connected Arduino on: " + deviceName);
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

    _appModel->allocateARDRawPins(2);

    _ard.sendAnalogPinReporting(0, ARD_ANALOG);	// AB: report data
	_ard.sendAnalogPinReporting(1, ARD_ANALOG);	// AB: report data
	setState(kARDCONTROLLER_READY);

}

void ArdController::updateArduino() {

    //LOG_VERBOSE("Update Ard");

    _ard.update();

    int * ardRawPins = _appModel->getARDRawPins();

    ardRawPins[0] = _ard.getAnalog(0);
    ardRawPins[1] = _ard.getAnalog(1);

    //LOG_VERBOSE("[" + ofToString(ardRawPins[0]) + "::" + ofToString(ardRawPins[1]) + "]");

}
