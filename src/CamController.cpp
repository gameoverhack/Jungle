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
    _lastFaceTime = _lastSwapTime = _lastTwitchyFaceTime = ofGetElapsedTimeMillis() - 60000;

    _camROI = new ofRectangle();

	// TODO: Possible bug here, doesn't check if loadClass was successful.
    if(loadClass("camROI" + ofToString(_instanceID) + ".bin", _camROI) == false){
    	LOG_WARNING("Could not load camera ROI for cam instance " + ofToString(_instanceID) + ", using sensible defaults.");
    	// Loading saved data failed, so we'll just make a best guess
		_camROI->x       = _camROI->y = 200.0f;
		_camROI->width   = 740.0f;
		_camROI->height  = 740.0f;
    }

    ofRegisterMouseEvents(this);

	LOG_NOTICE("Initialisation complete. Instance ID: " + ofToString(_instanceID));

}

//--------------------------------------------------------------
CamController::~CamController() {
	LOG_NOTICE("Destruction");
	//saveAttributes();
	stopThread();
	_cam.close();
}

bool CamController::setup(int deviceID, int w, int h){
    _isCamInit = false;
    string instanceMovieNames[2] = {"FakeCamFeedAttacker.mov", "FakeCamFeedVictim.mov"};

#ifdef USE_DUMMY
    string fakePathBase = boost::any_cast<string>(_appModel->getProperty("fakeDataPath"));
    LOG_NOTICE("Attempting to set instance " + ofToString(_instanceID) + " cam to DUMMY: " + fakePathBase+"/"+instanceMovieNames[_instanceID]);
    _cam.setPixelType(GO_TV_RGB);

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

    _doFaceDetection = true;
    _doFaceTracking = false;
    //_finder.setScaleHaar(0.5);
    _finder.setup("haarcascade_frontalface_default.xml");
    //_finder.setNeighbors(4);
    //_finder.setScaleHaar(1.09);
    _camImage.allocate(w, h);
    _colourImage.allocate(740,740);
    _greyImage.allocate(740,740);

    _camImage.setROI(_camROI->x, _camROI->y, _camROI->width, _camROI->height);

    _tracker.setup();
    _tracker.setScale(0.33);

    _width  = w;
    _height = h;

    startThread(false, false);

    return _isCamInit;
}

#ifdef TARGET_OSX
bool CamController::setup(string deviceID, int w, int h){

	LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to deviceID: " + deviceID);

	_cam.close();					// to be sure, to be sure
	_cam.setDeviceID(deviceID);
	_cam.initGrabber(w, h, true);

    //loadAttributes();

    _doFaceDetection = true;
    _doFaceTracking = false;
    //_finder.setScaleHaar(0.5);
    _finder.setup("haarcascade_frontalface_default.xml");
    _finder.setNeighbors(4);
    _finder.setScaleHaar(1.09);
    _camImage.allocate(w, h);
    _colourImage.allocate(740,740);
    _greyImage.allocate(740,740);

    _camImage.setROI(_camROI->x, _camROI->y, _camROI->width, _camROI->height);

    _tracker.setup();
    _tracker.setScale(1);

    _width  = w;
    _height = h;

    startThread(false, false);

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

#ifndef FORCE_FACE
    if (ofGetElapsedTimeMillis() - _lastFaceTime < TIMEOUT_NOFACE) {
        if (!_appModel->getFacePresent(_instanceID)) {  // arrived
            int actionType = _instanceID + 2;   // 0,1 leaving, 2,3 arriving
            _appModel->setFacePresent(_instanceID, true);
            //ofNotifyEvent(faceAction, actionType, this);
        }
    } else {                                            // gone
         if (_appModel->getFacePresent(_instanceID)) {
             int actionType = _instanceID + 0;  // 0,1 leaving, 2,3 arriving
             _appModel->setFacePresent(_instanceID, false);
            //ofNotifyEvent(faceAction, actionType, this);
         }
    }
    if (ofGetElapsedTimeMillis() - _lastFaceTime < TIMEOUT_SWAPFACE) {
        if (!_appModel->getSwapFacePresent(_instanceID)) {  // arrived
            _appModel->setSwapFacePresent(_instanceID, true);
        }
    } else {                                            // gone
         if (_appModel->getSwapFacePresent(_instanceID)) {
             _appModel->setSwapFacePresent(_instanceID, false);
         }
    }
#else
    _appModel->setFacePresent(_instanceID, FORCE_FACE);
    _appModel->setSwapFacePresent(_instanceID, FORCE_FACE);
#endif
}

void CamController::threadedFunction() {

    while(isThreadRunning() != 0) {
        if(lock()){

            if (_cam.isFrameNew() && _doFaceDetection) { //&& ofGetFrameNum()%2 == 0
                _camImage = _cam.getPixels();
                _greyImage = _camImage;
                if (_instanceID == 0) cvFlip(_greyImage.getCvImage(), _greyImage.getCvImage(), 1);
                cvTranspose(_greyImage.getCvImage(), _greyImage.getCvImage()); // rotate ROI

                if(_finder.findHaarObjects(_greyImage, 600, 600) > 0) { // has a face
                    _lastTwitchyFaceTime = ofGetElapsedTimeMillis();
                    //_lastFaceTime = ofGetElapsedTimeMillis();
                }

            }

            if (_cam.isFrameNew() && _doFaceTracking) {
                _camImage = _cam.getPixels();
                _colourImage = _camImage;
                if (_instanceID == 0) cvFlip(_colourImage.getCvImage(), _colourImage.getCvImage(), 1);
                cvTranspose(_colourImage.getCvImage(), _colourImage.getCvImage()); // rotate ROI

                if (_tracker.update(toCv(_colourImage))) { // has a face
                    _lastTwitchyFaceTime = ofGetElapsedTimeMillis();
                    //_lastFaceTime = ofGetElapsedTimeMillis();
                }
            }

            if (ofGetElapsedTimeMillis() - _lastTwitchyFaceTime < TIMEOUT_TWITCHYFACE) {
                _lastFaceTime = ofGetElapsedTimeMillis();
            }

        }
    }

//                Mat originalMat = Mat(_camImage.getHeight(), _camImage.getWidth(), getCvImageType(_camImage), _camImage.getPixels(), 0);
//                Mat rotatedMat =  Mat(_colourImage.getHeight(), _colourImage.getWidth(), getCvImageType(_colourImage), _colourImage.getPixels(), 0);
//
//                Point2f center(originalMat.rows / 2, originalMat.cols / 2);
//                Mat rotationMatrix = getRotationMatrix2D(center, 90.0f, 1);
//                warpAffine(originalMat, rotatedMat, rotationMatrix, originalMat.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0));
//                cvMat* matT, hdr;
//                matT = cvReshape( mat, &hdr, CV_MAT_CN(mat->type), mat->cols );
}

void CamController::drawDebug(float x, float y, float width, float height) {

    _xROIDisplay = x;
    _yROIDisplay = y,
    _xScaleROIDisplay = width/740.0f;
    _yScaleROIDisplay = height/740.0f;

    //if(!lock()){
        glPushMatrix();
        glScalef(_xScaleROIDisplay, _yScaleROIDisplay, 1.0f);
        glTranslatef(_xROIDisplay, _yROIDisplay, 0.0f);
        ofSetColor(255, 255, 255, 255);
        if (_doFaceTracking) {
            _colourImage.draw(0,0);
            _tracker.draw();
        }
        if (_doFaceDetection) {
            _greyImage.draw(0,0);
            glPushMatrix();
            glScalef(740.0f/_finder.getWidth(), 740.0f/_finder.getHeight(), 1.0f);
            _finder.draw(0,0);
            glPopMatrix();
        }

        if (_appModel->getFacePresent(_instanceID)) {
            ofFill();
            ofSetColor(0, 255, 0);
            ofCircle(25, 25, 50);
            ofNoFill();
            ofSetColor(255, 255, 255, 255);
        }
        glPopMatrix();
    //}
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
    if (_doROIAdjust && boost::any_cast<bool>(_appModel->getProperty("showCameras"))) {
        _camImage.setROI((_camROI->x + (_startX - e.x) * 2.0), (_camROI->y + (_startY - e.y) * 2.0), 740, 740);
    }
}

void CamController::mousePressed(ofMouseEventArgs &e) {
    if (e.x > _xROIDisplay * _xScaleROIDisplay && e.x < (_xROIDisplay + 740.0f) * _xScaleROIDisplay &&
        e.y > _yROIDisplay * _yScaleROIDisplay && e.y < (_yROIDisplay + 740.0f) * _yScaleROIDisplay &&
        boost::any_cast<bool>(_appModel->getProperty("showCameras"))) {
        _startX = e.x;
        _startY = e.y;
        ofRectangle r = _camImage.getROI();
        _camROI->x = r.x;
        _camROI->y = r.y;
        _camROI->width = r.width;
        _camROI->height = r.height;
        _doROIAdjust = true;
    }
}

void CamController::mouseReleased(ofMouseEventArgs &e) {
    if (_doROIAdjust && boost::any_cast<bool>(_appModel->getProperty("showCameras"))) {
        _doROIAdjust = false;
        _startX = _startY = -1;
        saveClass("camROI" + ofToString(_instanceID) + ".bin", _camROI);
    }
}
