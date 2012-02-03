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

void DebugView::update() {

    /********************************************************
     *  Set pointers for current Scene, Sequence Movie etc  *
     ********************************************************/

	Sequence		* currentSequence	= _appModel->getCurrentSequence();
	Scene			* currentScene		= _appModel->getCurrentScene();
    int             * pinInput          = _appModel->getPinInput();
    float attackLevel                   = _appModel->getARDAttackLevel();
    float min                           = boost::any_cast<float>(_appModel->getProperty("ardAttackMin"));
    float max                           = boost::any_cast<float>(_appModel->getProperty("ardAttackMax"));
    float delta                         = _appModel->getARDAttackDelta();
    float fftArea                       = _appModel->getFFTArea();

    bool showProps                      = boost::any_cast<bool>(_appModel->getProperty("showProps"));
    bool showFFT                        = boost::any_cast<bool>(_appModel->getProperty("showFFT"));

    string msg = "DEBUG\n";

    /********************************************************
     *        Print info for Mic and Ard Controllers        *
     ********************************************************/

    msg += "mic RAW area: " + ofToString(fftArea) + "\n";
    msg += "ard RAW pin0: " + ofToString(pinInput[0]) + " min: " + ofToString(min) + " max: " + ofToString(max) + "\n";
    msg += "ard RAW pin1: " + ofToString(pinInput[1]) + "\n";
    msg += "ard ATK levl: " + ofToString(attackLevel) + " delta: " + ofToString(delta) + "\n";

	// get frame rate
	msg += "FPS: " + ofToString(ofGetFrameRate()) + "\n";

	msg += "\n";
	msg += "Important keys: \n";
	msg += "Unmask heads: 'h'\nShow camera feeds: 'b'\nToggle FaceTracking/FaceDetection: 'w'\n";
	msg += "Select head to reposition: '0' and '1'\Select adjust Position: '5'\n";
	msg += "Select adjust ScaleRotation: '6'\nModify Position/ScaleRotation: Left,Right,Up,Down\n";
	msg += "Save head adjustments: 'c'\n";
	msg += "Swap cameras: 'n'\nAlter camera video settings: '3' and '4'\n";
	msg += "Pause scene: 'm'\nNext scene: 'j'\nGo back to start of scene: ' '\n";
	msg += "Fake victim action: 'q'\nFake attack action: 'p'\n";

    /********************************************************
     *              Print info for Transforms               *
     ********************************************************/

	// get transform info and current movie frame
	int sequenceCurrentFrame	= _appModel->getCurrentSequenceFrame(); //currentMovie->getCurrentFrame();
	int sequenceTotalFrames		= _appModel->getCurrentSequenceNumFrames(); //currentMovie->getTotalNumFrames();
	int sceneCurrentFrame       = _appModel->getCurrentSceneFrame();
	int sceneTotalFrames        = _appModel->getCurrentSceneNumFrames();

	//currentFrame = CLAMP(currentFrame, 0, totalFrames-1); // why are you so cruel?
    if (showProps) {

        msg += "vic1 Transform: " + currentSequence->getTransformAsString("vic1", sequenceCurrentFrame) + "\n";
        msg += "atk1 Transform: " + currentSequence->getTransformAsString("atk1", sequenceCurrentFrame) + "\n";
        if (currentSequence->getTransformCount() > 2) {
            msg += "atk2 Transform: " + currentSequence->getTransformAsString("atk2", sequenceCurrentFrame) + "\n";
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

	ofDrawBitmapString(msg, 20, 100);

    /********************************************************
     *               Draw Movie Progress Bar                *
     ********************************************************/

	// draw sequence progression
	float progressionPercentage = (float)sequenceCurrentFrame/(float)(sequenceTotalFrames);
	float progressionWidth = 300;
	float progressionHeight = 20;

    // draw the outline of the bar
	ofSetColor(50, 50, 50, 220);

	ofNoFill();
	ofRect(5, progressionHeight+10, progressionWidth+4, progressionHeight+2);

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
	ofRect(6, progressionHeight+9, progressionWidth*progressionPercentage, progressionHeight);
    ofNoFill();

    	// give us info about progression
	msg = currentScene->getName() + "::" + currentSequence->getName() + "::" + ofToString(sequenceCurrentFrame) + "/" + ofToString(sequenceTotalFrames);

	ofSetColor(255, 255, 255, 255);
	ofDrawBitmapString(msg, 6, progressionHeight+10+12);

    // draw total scene progression
	float totalProgressionPercentage = (float)sceneCurrentFrame/(float)(sceneTotalFrames);
	float totalProgressionWidth = 300;
	float totalProgressionHeight = 20;

    // draw the outline of the bar
	ofSetColor(50, 50, 50, 220);
	ofNoFill();
	ofRect(5, totalProgressionHeight+30, totalProgressionWidth+4, totalProgressionHeight+2);

    // draw the actual progress
    ofSetColor(50, 50, 220, 220);
    ofFill();
	ofRect(6, totalProgressionHeight+29, totalProgressionWidth*totalProgressionPercentage, totalProgressionHeight);
    ofNoFill();

    	// give us info about progression
	msg = currentScene->getName() + "::" + currentSequence->getName() + "::" + ofToString(sceneCurrentFrame) + "/" + ofToString(sceneTotalFrames);

	ofSetColor(255, 255, 255, 255);
	ofDrawBitmapString(msg, 6, totalProgressionHeight+progressionHeight+30+12);

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
		ofSetColor(255, 255, 255, 255);

        glTranslatef(ofGetWidth()/4.0f, 300.0f, 0.0f);
		plot(audioInput, bufferSize, plotHeight / 2, 0);

		glTranslatef(0.0f, plotHeight, 0.0f);
		plotDraw(fftInput, bufferSize, plotHeight, binSize);

        ofSetColor(255, 0, 0);
		plotDraw(fftNoiseFloor, bufferSize, plotHeight, binSize);

		ofSetColor(0, 0, 255, 255);
		plotDraw(fftCyclicSum, bufferSize, plotHeight, binSize, 1.0f, 1.0f/(float)binSize);

		ofSetColor(0, 255, 255, 255);
        plotDraw(fftPostFilter, bufferSize, plotHeight, binSize);

        glPushMatrix();

        glTranslatef(0.0f, plotHeight*3, 0.0f);

		//glRotatef(-(mouseY/ofGetHeight())*360.0f, fftBandSamples, 0.0f, 0.0f);
		//glRotatef((mouseX/ofGetWidth())*360.0f, 0.0f, fftBandSamples, 0.0f);

		for (int i = 0; i < cyclicBufferSize; i++) {

			ofSetColor(0, 255, 255, 255);
			glTranslatef(0.0f, 0.0f, 2.0f);
			plotDraw(fftCyclicBuffer[i].fftBand, bufferSize, plotHeight, binSize);

		}

    }

	glPopMatrix();
	_viewFBO.end();
}

void DebugView::plotDraw(float* array, int width, int height, int binSize, float scaleW, float scaleH) {

	glPushMatrix();
	glTranslatef(0, -height, 0);
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
	//glTranslatef(0, length + offset, 0);
	ofBeginShape();
	for (int i = 0; i < length; i++)
		ofVertex(i, array[i] * scale);
	ofEndShape();
	glPopMatrix();
}
