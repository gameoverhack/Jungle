/*
 *  ArdController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "ArdController.h"
//#include <signal.h>

//void handleSignal(int err){
//    cout << "Caught Signal Error: " << err << endl;
//    //abort();
//}

LONG WINAPI UnhandledExceptionCallback(PEXCEPTION_POINTERS pExceptPtrs) {
    if (IsDebuggerPresent()) {
        // Allow normal crash handling, which means the debugger will take over.
        cout << "WE HAD AN UNHANDLED EXCEPTION (DEBUG) - AND WE JUST DON'T CARE!" << endl;
        return EXCEPTION_CONTINUE_SEARCH;
    } else {
        cout << "WE HAD AN UNHANDLED EXCEPTION (RUNTIME) - AND WE JUST DON'T CARE!" << endl;
        cout << "Well we care but after 8 hours of looking at why ofSerial/ofArduino crash on exit, I don't care. Bye!" << endl;
        // Say we've handled it, so that the standard crash dialog is inhibited.
        return EXCEPTION_EXECUTE_HANDLER;
    }
}

ArdController::ArdController(string deviceName, int ardBufferLengthSecs) {

    LOG_NOTICE("Constructing ArdController");

    // I simply cannot get ofSerial/ofArduino to exit cleanly...so...

    // ...nasty way to catch exceptions when running in release...(signals method)
    //if(signal(SIGSEGV, handleSignal) == SIG_ERR) LOG_ERROR("Cannot handle SIGSEGV");

    // ...another nasty way to do the same on Windows:
    // 1) don't open a general purpose error message box,
    // 2) and handle the sigsegv using more windows API
    // see: http://stackoverflow.com/questions/3416413/can-one-prevent-microsoft-error-reporting-for-a-single-app
    //SetErrorMode(SEM_NOGPFAULTERRORBOX); // stops the error window
    //SetUnhandledExceptionFilter(UnhandledExceptionCallback); // catches the unhandled exception

    registerStates();

    _appModel->allocatePinInput(4);

    _ardCyclicBufferOffset = 0;
    _leftProximityHistory.resize(10);
    _rightProximityHistory.resize(10);

    #ifndef DONT_USE_ARDUINO
    if(!_ard.connect(deviceName, 57600)) {
        // could not connect to arduino
        setState(kARDCONTROLLER_DISABLED);
        LOG_ERROR("Cannot start Arduino on: " + deviceName);
        // fail to run if we can't get a hook into the arduino (EG: in production)
		abort();
	} else {
        LOG_NOTICE("Successfully connected Arduino on: " + deviceName);
		setState(kARDCONTROLLER_INIT);
    }
    #else
    setState(kARDCONTROLLER_DISABLED);
    LOG_ERROR("Continuing without arduino set to DISABLED");
    #endif
}

ArdController::~ArdController() {
    LOG_NOTICE("Disconnecting Arduino");
    if(!checkState(kARDCONTROLLER_DISABLED)) stop();
}

void ArdController::stop() {
    LOG_NOTICE("Stopping Arduino");
    setState(kARDCONTROLLER_DISABLED);
    _leftProximityHistory.clear();
    _rightProximityHistory.clear();
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
    _ard.sendAnalogPinReporting(2, ARD_ANALOG);	// AB: report data
    _ard.sendAnalogPinReporting(3, ARD_ANALOG);	// AB: report data

    ofSleepMillis(1000); // oh dear a magic number...does this stop the magic crashes??

	setState(kARDCONTROLLER_READY);

}

void ArdController::updateArduino(bool fake) {

    //LOG_VERBOSE("Update Ard");
    if (checkState(kARDCONTROLLER_DISABLED)) return;

    int * pinInput              = _appModel->getPinInput();

    if (!fake) {

        _ard.update();

        pinInput[0] = _ard.getAnalog(0);
        pinInput[1] = _ard.getAnalog(1);
        //pinInput[2] = _ard.getAnalog(2);
        //pinInput[3] = _ard.getAnalog(3);

    }

    _leftProximityHistory[_ardCyclicBufferOffset] = _ard.getAnalog(2);
    _rightProximityHistory[_ardCyclicBufferOffset] = _ard.getAnalog(3);

    float leftProximityAverage = 0;
    float rightProximityAverage = 0;

    for(int i = 0; i < _leftProximityHistory.size(); i++){
        leftProximityAverage += _leftProximityHistory[i];
    }

    leftProximityAverage = leftProximityAverage/_leftProximityHistory.size();

    for(int i = 0; i < _rightProximityHistory.size(); i++){
        rightProximityAverage += _rightProximityHistory[i];
    }

    rightProximityAverage = rightProximityAverage/_rightProximityHistory.size();

    pinInput[2] = leftProximityAverage;
    pinInput[3] = rightProximityAverage;

    if(leftProximityAverage > 100) {
        _appModel->restartTimer("anyFaceTimer");
        _appModel->restartTimer("victimFaceTimer");
    }
    if(rightProximityAverage > 100) {
        _appModel->restartTimer("anyFaceTimer");
        _appModel->restartTimer("attackFaceTimer");
    }

    //cout << leftProximityAverage << " " << rightProximityAverage << endl;

    _ardCyclicBufferOffset++;
    if(_ardCyclicBufferOffset > 10) _ardCyclicBufferOffset = 0;

    float level = 0.0f;

    float min = boost::any_cast<float>(_appModel->getProperty("ardAttackMin"));
    float max = boost::any_cast<float>(_appModel->getProperty("ardAttackMax"));

    float rawInput = CLAMP(pinInput[0], min, max);
    level = (( (max - rawInput) / (max - min) ) - 0.1) * 1.4;

    float lastLevel = _appModel->getARDAttackLevel();
    float delta = level - lastLevel;
    _appModel->setARDAttackDelta(delta);

    if (delta < 0) {
        float decrement = 0.04f;
        _appModel->setARDAttackLevel(lastLevel - decrement);
    } else {
        if (_appModel->getCurrentInteractivity() == kINTERACTION_BOTH ||
            _appModel->getCurrentInteractivity() == kINTERACTION_ATTACKER) {
            if (delta > 0.05 && level > 0.2) _appModel->restartTimer("attackActionTimer");
            _appModel->setARDAttackLevel(level);
        }
    }

    if (level > 1.0f &&
        _appModel->checkState(kAPP_RUNNING) &&
        (_appModel->getCurrentInteractivity() == kINTERACTION_BOTH ||
         _appModel->getCurrentInteractivity() == kINTERACTION_ATTACKER)) {

        if (_appModel->hasTimedOut("anyActionTimer")) {
            LOG_VERBOSE("Send attack event: " + ofToString(level));
            _appModel->sendAttackEvent(level);
        }

    }

}

void ArdController::fakeAttackAction(float input) {

    //LOG_VERBOSE("Fake Attack Action " + ofToString(input));

    int * pinInput              = _appModel->getPinInput();

    pinInput[0] = input;
    pinInput[1] = input;

    updateArduino(true);

}

