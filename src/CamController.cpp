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

#ifdef TARGET_OSX
bool CamController::setup(string deviceID, int x, int y){
	LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to deviceID: " + deviceID);
	_cam.close();					// to be sure, to be sure
    // TODO: make this work on WINDOZE
	_cam.setDeviceID(deviceID);
	_cam.initGrabber(x, y, true);
}
#endif

#ifdef TARGET_WIN32
void CamController::showVideoSettings() {
    _cam.showSettingsWindow();
}

void CamController::loadSettings() {
    // fill maps with current (ie., default values)
    map<string, setting> camSettings    = _cam.getCameraSettings();
    map<string, setting> filterSettings = _cam.getFilterSettings();

    map<string, setting>::iterator it;

    // get property on the model for each prop's CurrentValue (camera settings)
    for (it = camSettings.begin(); it != camSettings.end(); it++) {
        setting s = it->second;
        LOG_VERBOSE(s.print());
        // force all settings to manual
        s.flags = 2;
        s.CurrentValue = boost::any_cast<int>(_appModel->getProperty(s.propName));
    }

    // get property on the model for each prop's CurrentValue (filter settings)
    for (it = filterSettings.begin(); it != filterSettings.end(); it++) {
        setting s = it->second;
        LOG_VERBOSE(s.print());
        // force all settings to manual
        s.flags = 2;
        s.CurrentValue = boost::any_cast<int>(_appModel->getProperty(s.propName));
    }

    // set to values
    _cam.setCameraSettings(camSettings);
    _cam.setFilterSettings(filterSettings);

}

void CamController::saveSettings() {

    // get properties from the camera and filters as a map
    map<string, setting> camSettings    = _cam.getCameraSettings();
    map<string, setting> filterSettings = _cam.getFilterSettings();

    map<string, setting>::iterator it;

    // set property on the model for each prop's CurrentValue (camera settings)
    for (it = camSettings.begin(); it != camSettings.end(); it++) {
        setting s = it->second;
        LOG_VERBOSE(s.print());
        _appModel->setProperty(s.propName, (int)s.CurrentValue);
    }

    // set property on the model for each prop's CurrentValue (filter settings)
    for (it = filterSettings.begin(); it != filterSettings.end(); it++) {
        setting s = it->second;
        LOG_VERBOSE(s.print());
        _appModel->setProperty(s.propName, (int)s.CurrentValue);
    }

}
#endif

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
