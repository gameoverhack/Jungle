/*
 *  CamController.cpp
 *  Jungle
 *
 *  Created by gameover on 7/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "CamController.h"

//--------------------------------------------------------------
CamController::CamController() {
	LOG_NOTICE("Initialising");
	_cam.listDevices();
	_instanceCount++;				// use instance counts to keep track of which cam belongs to which viewer - may be redundant??
	_instanceID = _instanceCount;
	LOG_NOTICE("Initialisation complete. Instance ID: " + ofToString(_instanceID));
}

//--------------------------------------------------------------
CamController::~CamController() {
	LOG_NOTICE("Destruction");
	_cam.close();
}

bool CamController::setup(int deviceID, int x, int y){
	LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to deviceID: " + ofToString(deviceID));
	_cam.close();					// to be sure, to be sure
	_cam.setDeviceID(deviceID);
	_cam.initGrabber(x, y, true);
}

bool CamController::setup(string deviceID, int x, int y){
	LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to deviceID: " + deviceID);
	_cam.close();					// to be sure, to be sure
#ifdef TARGET_OSX
    // TODO: make this work on WINDOZE
	_cam.setDeviceID(deviceID);
#else
    LOG_WARNING("You need to write some code to actually select cam by string form of device ID on Windows 7!! For now ignoring...");
#endif
	_cam.initGrabber(x, y, true);
}

void CamController::update() {
	_cam.update();
}

int CamController::getInstanceID(){							// might be redundant
	return _instanceID;
}

void CamController::setInstanceID(int instanceID){			// might be redundant
	LOG_NOTICE("Setting instance ID to: " + instanceID);
	_instanceID = instanceID;
}

ofTexture * CamController::getCamTextureRef() {				// Ugggh a ViewTroller!!! ;-)
	return &(_cam.getTextureReference());
}
