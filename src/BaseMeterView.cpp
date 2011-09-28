#include "BaseMeterView.h"

BaseMeterView::BaseMeterView(float width, float height) : BaseView(width, height) {

    LOG_NOTICE("Constructing BaseMeterView");

    // set up shader
	string vertPath = boost::any_cast<string>(_appModel->getProperty("shaderLevelVertPath"));
	string fragPath = boost::any_cast<string>(_appModel->getProperty("shaderLevelFragPath"));

	_shader.setup(ofToDataPath(vertPath), ofToDataPath(fragPath));

}

BaseMeterView::~BaseMeterView() {
    //dtor
}

void BaseMeterView::update(interaction_t interactionType) {

    /********************************************************
     *      Get _appModel Interactivity & Threshold         *
     ********************************************************/

    int   currentInteractivity  = _appModel->getCurrentInteractivity();
    bool  isInteractive         = (currentInteractivity == kINTERACTION_BOTH || currentInteractivity == interactionType);

    bool  isBlocked             = (_scaledInputLevel > 0.05f);   // make more complex soon ;-)

    int   totalNumSequences     = _appModel->getCurrentScene()->getNumOfSequences();
    int   thisNumSequence       = _appModel->getCurrentSequence()->getNumber();

    //float threshold             = _appModel->getCurrentSequence()->getThresholdLevel();//(3.0/(float)totalNumSequences)+((float)thisNumSequence/(float)totalNumSequences); // TODO: move to the model
    //float bar_y                 = (672.0 - 6*42.0f) - floor((float)thisNumSequence/(float)totalNumSequences*12.0f)*42.0f; // magic numbers...bad boy

    /********************************************************
     *      Draw the Meter to the ViewFBO                   *
     ********************************************************/

    drawMeterMask(_scaledInputLevel);

    _viewFBO.begin();
    glPushMatrix();

    glClearColor(0.0, 0.0, 0.0, 1.0); // black background no transparency
    glClear(GL_COLOR_BUFFER_BIT);

    if (isInteractive) {

        //if (_scaledInputLevel > threshold) ofSetColor(255, 0, 0, 255); // no tinting

        drawMeterBlend(_meter_x, _meter_y);

         ofSetColor(255, 255, 255, 255); // no tinting
        //_meter_level->draw(_bar_x, bar_y - 5);
       // _icon_on->draw(_icon_x, _icon_y);

    } else {

        float blend = 0.0f; // TODO: make prop

        drawMeterBlend(_meter_x, _meter_y, blend);

        ofSetColor(255.0f*blend, 255.0f*blend, 255.0f*blend, 255.0f*blend); // no tinting
        //_icon_off->draw(_icon_x, _icon_y);

        //if (isBlocked) _icon_bar->draw(_icon_x, _icon_y);

    }

    glPopMatrix();

    _viewFBO.end();

}

void BaseMeterView::drawMeterMask(float input) {

    /********************************************************
     *    Draw a rect in an FBO to mask the level meter     *
     ********************************************************/

    _maskFBO.begin();
    glPushMatrix();

    glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
    glClear(GL_COLOR_BUFFER_BIT);

	ofSetColor(255, 255, 255, 255); // no tinting
	ofFill();
	glTranslatef(0.0f, _maskFBO.getHeight(), 0.0f);
	glScalef(1.0f, -1.0f, 1.0f);
    ofRect(0, 0, _maskFBO.getWidth(), floor(input * _meterSteps) * _meterPixelsForStep);
    //ofNoFill();

    glPopMatrix();
    _maskFBO.end();

}

void BaseMeterView::drawMeterBlend(float x, float y, float blend) {

    /********************************************************
     *    Use a shader to mask the meter_on/off png         *
     *    with the mask rect drawn in the FBO above         *
     ********************************************************/

    _shader.begin();
    glPushMatrix();

	_shader.setTexture("textures[0]", _maskTex, 10);
	_shader.setTexture("textures[1]", *_meter_on, 11);
	_shader.setTexture("textures[2]", *_meter_off, 12);

    _shader.setUniform1f("level", blend);

    glTranslatef(x, y, 1.0f);

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 0);	glVertex2f(0, 0);
	glTexCoord2f(0, _maskTex.getHeight()); glVertex2f(0, _maskTex.getHeight());
	glTexCoord2f(_maskTex.getWidth(), 0); glVertex2f(_maskTex.getWidth(), 0);
	glTexCoord2f(_maskTex.getWidth(), _maskTex.getHeight()); glVertex2f(_maskTex.getWidth(), _maskTex.getHeight());
	glEnd();

	glPopMatrix();
	_shader.end();

}
