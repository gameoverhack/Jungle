/*
 *  AppController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "AppController.h"

//--------------------------------------------------------------
void AppController::setup() {
	
	// TODO: move to DataController class
	// for now just make one new sequence in 1 Scene
	Sequence * newSequence = new Sequence();
	newSequence->_name = "someSeqName";
	newSequence->_victimResult = "someOtherSeqNameA";
	newSequence->_attackerResult = "someOtherSeqNameB";
	
	newSequence->_sequenceVideo.loadMovie("/Users/gameover/Desktop/StrangerDanger/video/t_seq_01_all_alpha_embedded2.mov");
	
	loadVector(ofToDataPath("Tw Seq01_sg1_transform.bin"), newSequence->_atk1Transforms);
	loadVector(ofToDataPath("Tw Seq01_sg2_transform.bin"), newSequence->_atk2Transforms);
	loadVector(ofToDataPath("Tw Seq01_mm_transform.bin"), newSequence->_vicTransforms);
	
	// etc
	
	_appModel->setSequence(newSequence, 0);
	_appModel->setCurrentSequence("someSeqName", 0);		// should this be broken into setScene and setSequence??? This seems ok for now..
	_appModel->getCurrentSequence()->_sequenceVideo.play();	// i guess the model could handle this but safer to make it explicit...
	

	// setup propertie/vars we might want to save using simple/boost::any props on model
	_appModel->setProperty("shaderBlendRatio", 0.5f);
	_appModel->setProperty("shaderGammaCorrection", 2.0f);
	
	_appModel->setProperty("shaderVertPath", (string)("vertex"));
	_appModel->setProperty("shaderFragPath", (string)("multitexturemerge"));
	
	cout << _appModel->getAllPropsAsList() << endl;
	
	_appView = new AppView(1280, 720);
	
}

// TODO: move to DataController class
void AppController::loadVector(string filePath, vector< CamTransform > & vec) {
	cout << "Loading vec: " << filePath << endl;
	std::ifstream ifs(filePath.c_str());
	boost::archive::text_iarchive ia(ifs);
	ia >> vec;	
}

//--------------------------------------------------------------
void AppController::update() {
	_appView->update();
}

//--------------------------------------------------------------
void AppController::draw() {
	
	ofSetColor(255, 255, 255, 255);
	_appView->draw();

	string msg = "FPS: " + ofToString(ofGetFrameRate()) + "\n";
	msg += _appModel->getAllPropsAsList();
	
	ofSetColor(0, 255, 0, 255);
	ofDrawBitmapString(msg, 20, 20);
}

//--------------------------------------------------------------
void AppController::keyPressed(int key){
	
	float gamma;
	float blend;
	
	switch (key) {
		case 'x':
			gamma = boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection")) + 0.01;
			break;
		case 'z':
			gamma = boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection")) - 0.01;
			break;
		case 's':
			blend = boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio")) + 0.01;
			break;
		case 'a':
			blend = boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio")) - 0.01;
			break;
		default:
			break;
	}
	
	_appModel->setProperty("shaderBlendRatio", gamma);
	_appModel->setProperty("shaderGammaCorrection", blend);
	
}

//--------------------------------------------------------------
void AppController::keyReleased(int key){
	
}

//--------------------------------------------------------------
void AppController::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void AppController::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void AppController::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void AppController::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void AppController::windowResized(int w, int h){
	
}

void AppController::setWindowed() {
	
}

void AppController::setFullscreen() {
	
}

void AppController::toggleFullscreen(){
	
}

