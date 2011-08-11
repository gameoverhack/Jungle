#include "AttackView.h"

AttackView::AttackView(float width, float height) : BaseView(width, height){
	// nothing?
}

void AttackView::update() {

    int * ardRawPins = _appModel->getARDRawPins();

    _viewFBO.begin();

    if(boost::any_cast<bool>(_appModel->getProperty("showDebugView")) && ardRawPins != NULL) {

        glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
        glClear(GL_COLOR_BUFFER_BIT);

        string msg;
        msg += "ard RAW pin0: " + ofToString(ardRawPins[0]) + "\n";
        msg += "ard RAW pin1: " + ofToString(ardRawPins[1]);

        // draw strings
        ofSetColor(0, 255, 255, 255);
        ofDrawBitmapString(msg, ofGetWidth() - 200, 20); // this gets rendered upside down for some reason ?

    }

    _viewFBO.end();

}
