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

	//_cam.listDevices();

    _lastFaceTimeTillLost   = 10000;
    _lastFaceTime           = ofGetElapsedTimeMillis() - _lastFaceTimeTillLost;

    _camROI.x       = _camROI.y = 200.0f;
    _camROI.width   = 640.0f;
    _camROI.height  = 640.0f;

    ofRegisterMouseEvents(this);

	_instanceID = _instanceCount;
    _instanceCount++;				// use instance counts to keep track of which cam belongs to which viewer - may be redundant??

	LOG_NOTICE("Initialisation complete. Instance ID: " + ofToString(_instanceID));

}

//--------------------------------------------------------------
CamController::~CamController() {
	LOG_NOTICE("Destruction");
	stopThread();
	_cam.close();
}

bool CamController::setup(int deviceID, int w, int h){



#ifdef USE_DUMMY
    LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to DUMMY");
    _cam.setPixelType(GO_TV_RGB);
    if (_instanceID == 0) _cam.loadMovie("E:/gameoverload/VideoProjects/Jungle/newtease/dummy/attackerCamBIG.mov");
    if (_instanceID == 1) _cam.loadMovie("E:/gameoverload/VideoProjects/Jungle/newtease/dummy/victimCamBIG.mov");
    _cam.play();
    ofSleepMillis(200);
#else
    LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to deviceID: " + ofToString(deviceID));
    _cam.close();// to be sure, to be sure
    bool ok = true; // bad mac change this
#ifdef TARGET_WIN32
    _cam.setRequestedMediaSubType(VI_MEDIASUBTYPE_MJPG);
	ok = _cam.initGrabber(w, h, true);
#else
    _cam.initGrabber(w, h, true);
#endif

	_cam.setDeviceID(deviceID);

#ifdef TARGET_WIN32
	if (ok) loadSettings();
#endif
#endif

    loadAttributes();

    _doFaceDetection = true;
    _doFaceTracking = false;
    //_finder.setScaleHaar(0.5);
    _finder.setup("haarcascade_frontalface_default.xml");
    //_finder.setNeighbors(4);
    //_finder.setScaleHaar(1.09);
    _camImage.allocate(w, h);
    _colourImage.allocate(640,640);
    _greyImage.allocate(640,640);

    _camImage.setROI(_camROI.x, _camROI.y, _camROI.width, _camROI.height);

    _tracker.setup();
    _tracker.setScale(0.33);

    _width  = w;
    _height = h;

    startThread(false, false);

    return ok;
}

#ifdef TARGET_OSX
bool CamController::setup(string deviceID, int w, int h){

	LOG_NOTICE("Attemptimg to set instance " + ofToString(_instanceID) + " cam to deviceID: " + deviceID);

	_cam.close();					// to be sure, to be sure
	_cam.setDeviceID(deviceID);
	_cam.initGrabber(w, h, true);

    loadAttributes();

    _doFaceDetection = true;
    _doFaceTracking = false;
    //_finder.setScaleHaar(0.5);
    _finder.setup("haarcascade_frontalface_default.xml");
    _finder.setNeighbors(4);
    _finder.setScaleHaar(1.09);
    _camImage.allocate(w, h);
    _colourImage.allocate(640,640);
    _greyImage.allocate(640,640);

    _camImage.setROI(_camROI.x, _camROI.y, _camROI.width, _camROI.height);

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

    // fill maps with current (ie., default values)

    map<string, setting> camSettings    = _cam.getCameraSettings();
    map<string, setting> filterSettings = _cam.getFilterSettings();

    map<string, setting> newCamSettings;
    map<string, setting> newFilterSettings;

    map<string, setting>::iterator it;

    // get property on the model for each prop's CurrentValue (camera settings)
    for (it = camSettings.begin(); it != camSettings.end(); it++) {

        setting s = it->second;

        if (_appModel->hasProperty(s.propName)) {

            setting n;

            LOG_VERBOSE("CURRENT SETTING: " + s.print());

            // copy props to new n setting
            n.propName      = s.propName;
            n.propID        = s.propID;
            n.min           = s.min;
            n.max           = s.max;
            n.SteppingDelta = s.SteppingDelta;
            n.flags         = 2;            // force all settings to manual
            n.CurrentValue  = boost::any_cast<int>(_appModel->getProperty(n.propName + "_" + ofToString(_instanceID)));

            LOG_VERBOSE("CHANGETO SETTING: " + n.print());

            // stor in new settings map
            newCamSettings.insert(pair<string, setting>(n.propName, n));
        }

    }

    // get property on the model for each prop's CurrentValue (filter settings)
    for (it = filterSettings.begin(); it != filterSettings.end(); it++) {

        setting s = it->second;

         if (_appModel->hasProperty(s.propName)) {

            setting n;

            LOG_VERBOSE("CURRENT SETTING: " + s.print());

            // copy props to new n setting
            n.propName      = s.propName;
            n.propID        = s.propID;
            n.min           = s.min;
            n.max           = s.max;
            n.SteppingDelta = s.SteppingDelta;
            n.flags         = 2;            // force all settings to manual
            n.CurrentValue  = boost::any_cast<int>(_appModel->getProperty(n.propName + "_" + ofToString(_instanceID)));

            LOG_VERBOSE("CHANGETO SETTING: " + n.print());

            // stor in new settings map
            newFilterSettings.insert(pair<string, setting>(n.propName, n));
         }
    }

    // set to values
     if (newCamSettings.size() > 0)     _cam.setCameraSettings(newCamSettings);
     if (newFilterSettings.size() > 0)  _cam.setFilterSettings(newFilterSettings);

    camSettings.clear();
    filterSettings.clear();
    newCamSettings.clear();
    newFilterSettings.clear();

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
        _appModel->setProperty(s.propName + "_" + ofToString(_instanceID), (int)s.CurrentValue);
    }

    // set property on the model for each prop's CurrentValue (filter settings)
    for (it = filterSettings.begin(); it != filterSettings.end(); it++) {
        setting s = it->second;
        LOG_VERBOSE(s.print());
        _appModel->setProperty(s.propName + "_" + ofToString(_instanceID), (int)s.CurrentValue);
    }

    camSettings.clear();
    filterSettings.clear();

     // save to drive here or just let it happen on quit??

}
#endif
#endif

void CamController::loadAttributes() {

    float camRotation           = boost::any_cast<float>(_appModel->getProperty("camRotation" + ofToString(_instanceID)));
    float camScale              = boost::any_cast<float>(_appModel->getProperty("camScale" + ofToString(_instanceID)));
    float camPositionX          = boost::any_cast<float>(_appModel->getProperty("camPositionX" + ofToString(_instanceID)));
    float camPositionY          = boost::any_cast<float>(_appModel->getProperty("camPositionY" + ofToString(_instanceID)));

    PosRotScale * prs = new PosRotScale();

    prs->x           = camPositionX;
    prs->y           = camPositionY;
    prs->r           = camRotation;
    prs->s           = camScale;

    setCameraAttributes(prs);

}

void CamController::saveAttributes() {

    PosRotScale * prs = _appModel->getCameraAttributes(_instanceID);

    _appModel->setProperty("camRotation" + ofToString(_instanceID), prs->r);
    _appModel->setProperty("camScale" + ofToString(_instanceID), prs->s);
    _appModel->setProperty("camPositionX" + ofToString(_instanceID), prs->x);
    _appModel->setProperty("camPositionY" + ofToString(_instanceID), prs->y);

    // save to drive here or just let it happen on quit??
}

void CamController::setCameraAttributes(PosRotScale * prs) {

    LOG_NOTICE("Saving [" + prs->print(false) + "]");

    _appModel->setCameraAttributes(_instanceID, prs);

}

void CamController::update() {

    _cam.update();
    if (ofGetElapsedTimeMillis() - _lastFaceTime < _lastFaceTimeTillLost) {
        if (!_appModel->getFacePresent(_instanceID)) {  // arrived
            int instanceID = _instanceID + 2;
            _appModel->setFacePresent(_instanceID, true);
            ofNotifyEvent(faceAction, instanceID, this);
        }
    } else {                                            // gone
         if (_appModel->getFacePresent(_instanceID)) {
             int instanceID = _instanceID + 0;
             _appModel->setFacePresent(_instanceID, false);
            ofNotifyEvent(faceAction, instanceID, this);
         }
    }
}

void CamController::threadedFunction() {

    while(isThreadRunning() != 0) {
        if(lock()){

            if (_cam.isFrameNew() && _doFaceDetection) { //&& ofGetFrameNum()%2 == 0
                _camImage = _cam.getPixels();
                _greyImage = _camImage;
                if (_instanceID == 1) cvFlip(_greyImage.getCvImage(), _greyImage.getCvImage(), 1);
                cvTranspose(_greyImage.getCvImage(), _greyImage.getCvImage()); // rotate ROI

                if(_finder.findHaarObjects(_greyImage, 200, 200) > 0) { // has a face
                    _lastFaceTime = ofGetElapsedTimeMillis();
                }

            }

            if (_cam.isFrameNew() && _doFaceTracking) {
                _camImage = _cam.getPixels();
                _colourImage = _camImage;
                if (_instanceID == 1) cvFlip(_colourImage.getCvImage(), _colourImage.getCvImage(), 1);
                cvTranspose(_colourImage.getCvImage(), _colourImage.getCvImage()); // rotate ROI

                if (_tracker.update(toCv(_colourImage))) { // has a face
                    _lastFaceTime = ofGetElapsedTimeMillis();
                }
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
    _xScaleROIDisplay = width/640.0f;
    _yScaleROIDisplay = height/640.0f;

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
            glScalef(640.0f/_finder.getWidth(), 640.0f/_finder.getHeight(), 1.0f);
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
        _camImage.setROI((_camROI.x + (_startX - e.x) * 2.0), (_camROI.y + (_startY - e.y) * 2.0), 640, 640);
    }
}

void CamController::mousePressed(ofMouseEventArgs &e) {
    if (e.x > _xROIDisplay * _xScaleROIDisplay && e.x < (_xROIDisplay + 640.0f) * _xScaleROIDisplay &&
        e.y > _yROIDisplay * _yScaleROIDisplay && e.y < (_yROIDisplay + 640.0f) * _yScaleROIDisplay &&
        boost::any_cast<bool>(_appModel->getProperty("showCameras"))) {
        _startX = e.x;
        _startY = e.y;
        _camROI = _camImage.getROI();
        _doROIAdjust = true;
    }
}

void CamController::mouseReleased(ofMouseEventArgs &e) {
    _doROIAdjust = false;
    _startX = _startY = -1;
}
