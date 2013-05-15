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

	LOG_NOTICE("Constructing CamController");

    registerStates();

	_instanceID = _instanceCount;
    _instanceCount++;				// use instance counts to keep track of which cam belongs to which viewer - may be redundant??

	//_cam.listDevices();
    _isCamInit = false;

    ofRegisterMouseEvents(this);

	LOG_NOTICE("Initialisation complete. Instance ID: " + ofToString(_instanceID));

}

//--------------------------------------------------------------
CamController::~CamController() {
	LOG_NOTICE("Destruction");
	//saveAttributes();
	_cam.update();
	_cam.close();
}

bool CamController::setup(int deviceID, int w, int h){
    _isCamInit = false;
    string instanceMovieNames[2] = {"FakeCamFeedAttacker.mov", "FakeCamFeedVictim.mov"};

#ifdef USE_DUMMY
    string fakePathBase = boost::any_cast<string>(_appModel->getProperty("fakeDataPath"));
    LOG_NOTICE("Attempting to set instance " + ofToString(_instanceID) + " cam to DUMMY: " + fakePathBase+"/"+instanceMovieNames[_instanceID]);
    //_cam.setPixelType(GO_TV_RGB);

    // try to load up fake camera feed
    _isCamInit = _cam.loadMovie(fakePathBase +"/"+ instanceMovieNames[_instanceID]);
    if(_isCamInit){
        _cam.play();
        //_cam.setUseTexture(true);
        ofSleepMillis(200);
    }
    else{
        LOG_ERROR("Could not load movie: " + fakePathBase+"/"+instanceMovieNames[_instanceID]);
        abort();
    }
#else
    _cam.setDeviceID(deviceID);
    LOG_NOTICE("Attempting to set instance " + ofToString(_instanceID) + " cam to deviceID: " + ofToString(deviceID));
    //_cam.close();// to be sure, to be sure

    #ifdef TARGET_WIN32
    _cam.setRequestedMediaSubType(VI_MEDIASUBTYPE_MJPG);
	_isCamInit = _cam.initGrabber(w, h, true);
    #else
    _isCamInit = true; // bad mac doesn't return a bool on setup!! change this
    _cam.initGrabber(w, h, true);
	#endif
	#ifdef TARGET_WIN32
	loadSettings();
	#endif
#endif

    //loadAttributes();

    _width  = w;
    _height = h;

    return _isCamInit;
}

#ifdef TARGET_OSX
bool CamController::setup(string deviceID, int w, int h){

	LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to deviceID: " + deviceID);

	_cam.close();					// to be sure, to be sure
	_cam.setDeviceID(deviceID);
	_cam.initGrabber(w, h, true);

    //loadAttributes();

    _width  = w;
    _height = h;

    return true; //bad mac need to change grabber?
}
#endif

#ifndef USE_DUMMY
#ifdef TARGET_WIN32
void CamController::showVideoSettings() {
    _cam.showSettingsWindow();
}

void CamController::loadSettings() {

    if (_isCamInit) {

        LOG_NOTICE("Loading Camera Settings for instance: " + ofToString(_instanceID));

        Settings * cS = new Settings();
        Settings * fS = new Settings();
        bool loaded = false;

        loaded = loadClass(ofToDataPath("cameraSettings" + ofToString(_instanceID) + ".bin"), cS);
        if(!loaded){
            LOG_VERBOSE("Could not load camera settings for " + ofToString(_instanceID));
        }
        loaded = loadClass(ofToDataPath("filterSettings" + ofToString(_instanceID) + ".bin"), fS);
        if(!loaded){
            LOG_VERBOSE("Could not load camera settings for " + ofToString(_instanceID));
        }

        if (cS->settings.size() > 0) _cam.setCameraSettings(cS->settings);
        if (fS->settings.size() > 0) _cam.setFilterSettings(fS->settings);

        delete cS;
        delete fS;
    }

}

void CamController::saveSettings() {

    if (_isCamInit) {

        LOG_NOTICE("Saving Camera Settings for instance: " + ofToString(_instanceID));

        Settings * cS = new Settings();
        cS->settings = _cam.getCameraSettings();

        Settings * fS = new Settings();
        fS->settings = _cam.getFilterSettings();

        saveClass(ofToDataPath("cameraSettings" + ofToString(_instanceID) + ".bin"), cS);
        saveClass(ofToDataPath("filterSettings" + ofToString(_instanceID) + ".bin"), fS);

        delete cS;
        delete fS;

    }

}
#endif
#endif

void CamController::loadAttributes() {

    LOG_NOTICE("Loading Camera Attributes for instance: " + ofToString(_instanceID));

    Scene * currentScene = _appModel->getCurrentScene();

    bool loaded = false;

    string sceneName = currentScene->getName();
    string scenePath = boost::any_cast<string>(_appModel->getProperty("scenesDataPath")) + "/" + sceneName + "/" + sceneName + "_";

    PosRotScale * prsC = new PosRotScale();
    loaded = loadClass(scenePath + "cameraAttributes" + ofToString(_instanceID) + ".bin", prsC);
    if(!loaded){
        LOG_VERBOSE("Could not load camera settings for " + ofToString(_instanceID) + "(" + scenePath + "cameraAttributes" + ofToString(_instanceID) + ".bin" + ")");
    }

    PosRotScale * prsF = new PosRotScale();
    loaded = loadClass(scenePath + "fakeAttributes" + ofToString(_instanceID) + ".bin", prsF);
    if(!loaded){
        LOG_VERBOSE("Could not load camera settings for " + ofToString(_instanceID));
    }
    LOG_VERBOSE("ATTEMPT TO SET CAM+FAKE ATTRIBUTES FOR " + ofToString(_instanceID));
    setCameraAttributes(prsC);
    setFakeAttributes(prsF);
    LOG_VERBOSE("SET CAM+FAKE ATTRIBUTES FOR " + ofToString(_instanceID));

}

void CamController::saveAttributes() {

    LOG_NOTICE("Saving Camera Attributes for instance: " + ofToString(_instanceID));

    Scene * currentScene = _appModel->getCurrentScene();

    string sceneName = currentScene->getName();
    string scenePath = boost::any_cast<string>(_appModel->getProperty("scenesDataPath")) + "/" + sceneName + "/" + sceneName + "_";

    PosRotScale * prsC = _appModel->getCameraAttributes(_instanceID);
    saveClass(scenePath + "cameraAttributes" + ofToString(_instanceID) + ".bin", prsC);

    PosRotScale * prsF = _appModel->getFakeAttributes(_instanceID);
    saveClass(scenePath + "fakeAttributes" + ofToString(_instanceID) + ".bin", prsF);

}

void CamController::setCameraAttributes(PosRotScale * prs) {

    LOG_NOTICE("Setting Camera Attributes[" + prs->print(false) + "]");

    _appModel->setCameraAttributes(_instanceID, prs);

}

void CamController::setFakeAttributes(PosRotScale * prs) {

    LOG_NOTICE("Setting Fake Attributes [" + prs->print(false) + "]");

    _appModel->setFakeAttributes(_instanceID, prs);

}

void CamController::update() {
    _cam.update();
}

void CamController::drawDebug(float x, float y, float width, float height) {
    _cam.draw(x,y,width, height);
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

void CamController::mouseMoved(ofMouseEventArgs &e) {

}

void CamController::mouseDragged(ofMouseEventArgs &e) {

}

void CamController::mousePressed(ofMouseEventArgs &e) {

}

void CamController::mouseReleased(ofMouseEventArgs &e) {

}
