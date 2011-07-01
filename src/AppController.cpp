/*
 *  AppController.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "AppController.h"
#include "Logger.h"

//--------------------------------------------------------------
void AppController::setup() {
	LOGGER->setLogLevel(JU_LOG_NOTICE);
	LOG_NOTICE("Initialising");
	// TODO: move to DataController class
	// for now just make one new sequence in 1 Scene
	Sequence * newSequence = new Sequence();
	newSequence->_name = "someSeqName";
	newSequence->_victimResult = "someOtherSeqNameA";
	newSequence->_attackerResult = "someOtherSeqNameB";
	
	newSequence->_sequenceVideo.loadMovie("/Users/ollie/Source/of_62_osx/apps/stranger_danger_artifacts/t_seq_01_all_alpha_embedded2.mov");
	
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
	
	_appModel->setProperty("showDebugView", true);
	
	cout << _appModel->getAllPropsAsList() << endl;
	
	_appView = new AppView(1280, 720);
	LOG_NOTICE("Initialised");
	
}

// TODO: move to DataController class
void AppController::loadVector(string filePath, vector< CamTransform > & vec) {
	LOG_NOTICE(("Loading Vector: " + filePath));
//	cout << "Loading vec: " << filePath << endl;
	std::ifstream ifs(filePath.c_str());
	boost::archive::text_iarchive ia(ifs);
	ia >> vec;	
}

//--------------------------------------------------------------
void AppController::update() {
//	LOG_NOTICE("Updaing");
	_appView->update();
}

//--------------------------------------------------------------
void AppController::draw() {
//	LOG_NOTICE("Drawing");
	ofSetColor(255, 255, 255, 255);
	_appView->draw();
	
}

//--------------------------------------------------------------
void AppController::keyPressed(int key){
	
	float gamma = boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection"));
	float blend = boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio"));
	
	switch (key) {
		case 'x':
			gamma += 0.1;
			break;
		case 'z':
			gamma -= 0.1;
			break;
		case 's':
			blend += 0.1;
			break;
		case 'a':
			blend -= 0.1;
			break;
		case 'd':
			_appModel->setProperty("showDebugView", !boost::any_cast<bool>(_appModel->getProperty("showDebugView")));
			break;			
		default:
			break;
	}
	printf("Gamma: %f\n", gamma);
	_appModel->setProperty("shaderBlendRatio", blend);
	_appModel->setProperty("shaderGammaCorrection", gamma);
	
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

