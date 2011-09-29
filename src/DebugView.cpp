/*
 *  DebugView.cpp
 *  Jungle
 *
 *  Created by ollie on 30/06/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#include "DebugView.h"


DebugView::DebugView(float width, float height) : BaseView(width, height){
	LOG_NOTICE("Setting up DebugView");
}

void DebugView::update(){

    /********************************************************
     *  Set pointers for current Scene, Sequence Movie etc  *
     ********************************************************/

	Sequence		* currentSequence	= _appModel->getCurrentSequence();
	Scene			* currentScene		= _appModel->getCurrentScene();
    int             * pinInput          = _appModel->getPinInput();
    float             fftArea           = _appModel->getFFTArea();

    bool showProps                      = boost::any_cast<bool>(_appModel->getProperty("showProps"));
    bool showFFT                        = boost::any_cast<bool>(_appModel->getProperty("showFFT"));

    string            msg;

    /********************************************************
     *        Print info for Mic and Ard Controllers        *
     ********************************************************/

    msg += "mic RAW area: " + ofToString(fftArea) + "\n";
    msg += "ard RAW pin0: " + ofToString(pinInput[0]) + "\n";
    msg += "ard RAW pin1: " + ofToString(pinInput[1]) + "\n";

	// get frame rate
	msg += "ABC: " + ofToString(ofGetFrameRate()) + "\n";

    /********************************************************
     *              Print info for Transforms               *
     ********************************************************/

	// get transform info and current movie frame
	int currentFrame	= _appModel->getCurrentFrame(); //currentMovie->getCurrentFrame();
	int totalFrames		= _appModel->getCurrentFrameTotal(); //currentMovie->getTotalNumFrames();

	//currentFrame = CLAMP(currentFrame, 0, totalFrames-1); // why are you so cruel?
    if (showProps) {
        msg += "vic1 Transform: " + currentSequence->getTransformAsString("vic1", currentFrame) + "\n";
        msg += "atk1 Transform: " + currentSequence->getTransformAsString("atk1", currentFrame) + "\n";
        if (currentSequence->getTransformCount() > 2) {
            msg += "atk2 Transform: " + currentSequence->getTransformAsString("atk2", currentFrame) + "\n";
        }

        // get all the properties on the AppModel
        msg += _appModel->getAllPropsAsList();
    }

    /********************************************************
     *                      Draw ViewFBO                    *
     ********************************************************/

	_viewFBO.begin();
	glPushMatrix();

	glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
	glClear(GL_COLOR_BUFFER_BIT);

    /********************************************************
     *                    Draw MSG info                     *
     ********************************************************/

	ofSetColor(0, 255, 255, 255);
	ofDrawBitmapString(msg, 20, 50); // this gets rendered upside down for some reason ?*/

    /********************************************************
     *               Draw Movie Progress Bar                *
     ********************************************************/

	// draw scene progression
	float progressionPercentage = (float)currentFrame/(float)(totalFrames);
	float progressionWidth = 300;
	float progressionHeight = 20;

    // draw the outline of the bar
	ofSetColor(50, 50, 50, 220);
	ofNoFill();
	ofRect(5, _viewHeight-progressionHeight-10, progressionWidth+4, progressionHeight+2);

	// valid movie file/sequence stuff
	if(currentSequence->getIsSequenceFaked()) {
		ofSetColor(255, 50, 50, 220);
	}
	else if(currentSequence->getIsMovieFaked()) {
		ofSetColor(255, 120, 0, 220);
	} else {
		ofSetColor(50, 220, 50, 220);
	}

    // draw the actual progress
    ofFill();
	ofRect(6, _viewHeight-progressionHeight-9, progressionWidth*progressionPercentage, progressionHeight);
    ofNoFill();

	// give us info about progression
	msg = currentScene->getName() + "::" + currentSequence->getName() + "::" + ofToString(currentFrame) + "/" + ofToString(totalFrames);

	ofSetColor(255, 255, 255, 255);
	ofDrawBitmapString(msg, 6, _viewHeight-progressionHeight-4);

    if (showFFT) {

        /********************************************************
         *             Draw FFT MicController Data              *
         ********************************************************/

        fftBands * fftCyclicBuffer  = _appModel->getFFTCyclicBuffer();

        float * fftNoiseFloor       = _appModel->getFFTNoiseFloor();
        float * fftCyclicSum        = _appModel->getFFTCyclicSum();
        float * fftPostFilter       = _appModel->getFFTPostFilter();
        float * fftInput            = _appModel->getFFTInput();
        float * audioInput          = _appModel->getAudioInput();

        int bufferSize              = _appModel->getAudioBufferSize();
        int binSize                 = _appModel->getFFTBinSize();
        int cyclicBufferSize        = _appModel->getFFTCyclicBufferSize();
        int plotHeight              = 128;

        glPushMatrix();

        ofNoFill();

        glScalef(1.0f, -1.0f, 1.0);
        glTranslatef(1.0, -_viewHeight, 0.0f);
		ofSetColor(255, 255, 255, 255);

        glTranslatef(16.0f, 16.0f, 0.0f);
		ofDrawBitmapString("Frequency Domain", 0, plotHeight);      // upside down Mr Squiggle - totally screwy
		plot(audioInput, bufferSize, plotHeight / 2, 0);

		glTranslatef(0.0f, (float)plotHeight + 16.0f, 0.0f);
		ofDrawBitmapString("Time Domain", 0, plotHeight*2);         // upside down Mr Squiggle - totally screwy
		plotDraw(fftInput, bufferSize, plotHeight, binSize);

        ofSetColor(255, 0, 0);
		plotDraw(fftNoiseFloor, bufferSize, plotHeight, binSize);

		ofSetColor(0, 0, 255, 255);
		plotDraw(fftCyclicSum, bufferSize, plotHeight, binSize, 1.0f, 1.0f/(float)binSize);

		ofSetColor(0, 255, 255, 255);
		plotDraw(fftPostFilter, bufferSize, plotHeight, binSize);

        glPushMatrix();

        glTranslatef(bufferSize, (float)ofGetHeight()/2.0f, 0.0f);

		//glRotatef(-(mouseY/ofGetHeight())*360.0f, fftBandSamples, 0.0f, 0.0f);
		//glRotatef((mouseX/ofGetWidth())*360.0f, 0.0f, fftBandSamples, 0.0f);

		for (int i = 0; i < cyclicBufferSize; i++) {

			ofSetColor(0, 255, 255, 255);
			glTranslatef(0.0f, 0.0f, 2.0f);
			plotDraw(fftCyclicBuffer[i].fftBand, bufferSize, plotHeight, binSize);

		}

		glPopMatrix();

        glPopMatrix();

        /********************************************************
         *             Draw ARD ArdController Data              *
         ********************************************************/

        float * ardCyclicBuffer     = _appModel->getARDCyclicBuffer();
        int ardCycicBufferSize      = _appModel->getARDCyclicBufferSize();

        glPushMatrix();

        ofNoFill();

        //glScalef(1.0f, -1.0f, 1.0);
        //glTranslatef((float)ofGetWidth()/2.0f, -_viewHeight, 1.0);
		ofSetColor(255, 255, 255, 255);

        glTranslatef(16.0f, 16.0f, 0.0f);
		ofDrawBitmapString("Ard Cyclic Buffer", 0, plotHeight);      // upside down Mr Squiggle - totally screwy
		glTranslatef(0.0f, plotHeight, 0.0f);
		plotDraw(ardCyclicBuffer, ardCycicBufferSize, plotHeight, ardCycicBufferSize, 10.0f, 2.0f);

		glPopMatrix();

    }

	glPopMatrix();
	_viewFBO.end();
}

void DebugView::plotDraw(float* array, int width, int height, int binSize, float scaleW, float scaleH) {

	glPushMatrix();
	glTranslatef(0, height, 0);
	glScalef(width/binSize * scaleW, -scaleH, 0.0f);
	ofBeginShape();
	for (int i = 0; i < binSize; i++)
		ofVertex(i, array[i]*width);
	ofEndShape();
	glPopMatrix();

}

void DebugView::plot(float* array, int length, float scale, float offset) {
	//ofNoFill();
	//ofRect(0, 0, length, length*2.0);
	glPushMatrix();
	glTranslatef(0, length + offset, 0);
	ofBeginShape();
	for (int i = 0; i < length; i++)
		ofVertex(i, array[i] * scale);
	ofEndShape();
	glPopMatrix();
}
