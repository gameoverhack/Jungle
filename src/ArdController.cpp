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

    _appModel->allocatePinInput(2);

    if(!_ard.connect(deviceName, 57600)) {
        // could not connect to arduino
        setState(kARDCONTROLLER_DISABLED);

        LOG_ERROR("Cannot start Arduino on: " + deviceName);

        if(!boost::any_cast<bool>(_appModel->getProperty("ardIgnoreSetupFailure"))){
            // fail to run if we can't get a hook into the arduino (EG: in production)
            abort();
        } else {
            LOG_ERROR("Continuing without arduino set to DISABLED");
        }

    } else {

        LOG_NOTICE("Successfully connected Arduino on: " + deviceName);

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
    registerState(kARDCONTROLLER_DISABLED, "kARDCONTROLLER_DISABLED");
	registerState(kARDCONTROLLER_INIT, "kARDCONTROLLER_INIT");
	registerState(kARDCONTROLLER_READY, "kARDCONTROLLER_READY");

	setState(kARDCONTROLLER_RETARD);
}

void ArdController::update() {

    if (_ard.isArduinoReady() || checkState(kARDCONTROLLER_INIT)) {

		// 1st: setup the arduino if haven't already:
		if (checkState(kARDCONTROLLER_INIT)) {
			setupArduino();
			ofSleepMillis(500);
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

    _lastUpdateTime = ofGetElapsedTimeMillis();

	setState(kARDCONTROLLER_READY);

}

void ArdController::updateArduino(bool fake) {

    //LOG_VERBOSE("Update Ard");

    int * pinInput              = _appModel->getPinInput();

    if (!fake) {

        _ard.update();

        pinInput[0] = _ard.getAnalog(0);
        pinInput[1] = _ard.getAnalog(1);

    }

    float level = 0.0f;

    float min = boost::any_cast<float>(_appModel->getProperty("ardAttackMin"));
    float max = boost::any_cast<float>(_appModel->getProperty("ardAttackMax"));

    float rawInput = CLAMP(pinInput[0], min, max);
    level = (( (max - rawInput) / (max - min) ) - 0.1) * 1.4;

    float lastLevel = _appModel->getARDAttackLevel();
    float delta = floor(level - lastLevel);
    _appModel->setARDAttackDelta(delta);

    if (delta < 0) {
        float decrement = 0.04f;
        _appModel->setARDAttackLevel(lastLevel - decrement);
    } else {
        if (_appModel->getCurrentInteractivity() == kINTERACTION_BOTH || _appModel->getCurrentInteractivity() == kINTERACTION_ATTACKER)_appModel->setARDAttackLevel(level);
    }

    if (level > 1.0f && _appModel->checkState(kAPP_RUNNING) && (_appModel->getCurrentInteractivity() == kINTERACTION_BOTH || _appModel->getCurrentInteractivity() == kINTERACTION_ATTACKER)) {
        LOG_VERBOSE("Send attack event: " + ofToString(level));
        _appModel->sendAttackEvent(level);
    }

}

void ArdController::fakeAttackAction(float input) {

    //LOG_VERBOSE("Fake Attack Action " + ofToString(input));

    int * pinInput              = _appModel->getPinInput();

    pinInput[0] = input;
    pinInput[1] = input;

    updateArduino(true);

}

