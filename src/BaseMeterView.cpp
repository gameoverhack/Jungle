#include "BaseMeterView.h"

BaseMeterView::BaseMeterView(float width, float height) : BaseView(width, height) {

    LOG_NOTICE("Constructing BaseMeterView");

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    _meter_on      = _appModel->getGraphicTex(kGFX_METER_ON);
    _meter_off     = _appModel->getGraphicTex(kGFX_METER_OFF);
    _meter_level   = _appModel->getGraphicTex(kGFX_METER_LEVEL);

    /********************************************************
     *      setup meter FBO and Shader                      *
     ********************************************************/

    // allocate fbo's
    _maskTex.allocate(124, 672, GL_RGBA); // size of the meter_on/off png
    _maskFBO.setup(124, 672);
    _maskFBO.attach(_maskTex);

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
    bool  isInteractive         = true;//(currentInteractivity == kINTERACTION_BOTH || currentInteractivity == interactionType);

    bool  isBlocked             = (_scaledInputLevel > 0.2f);   // make more complex soon ;-)

    int   totalNumSequences     = _appModel->getCurrentScene()->getNumOfSequences();
    int   thisNumSequence       = _appModel->getCurrentSequence()->getNumber();

    float barThreshold          = (3.0/(float)totalNumSequences)+((float)thisNumSequence/(float)totalNumSequences); // TODO: move to the model
    float bar_y                 = (672.0 - 4*42.0f) - floor((float)thisNumSequence/(float)totalNumSequences*12.0f)*42.0f;

    /********************************************************
     *      Draw the Meter to the ViewFBO                   *
     ********************************************************/

    if (isInteractive) drawMeterMask(_scaledInputLevel);

    _viewFBO.begin();
    glPushMatrix();

    glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
    glClear(GL_COLOR_BUFFER_BIT);

    if (_scaledInputLevel > barThreshold) ofSetColor(255, 0, 0, 255); // no tinting

    _meter_level->draw(_bar_x, bar_y);

    ofSetColor(255, 255, 255, 255); // no tinting

    if (isInteractive) {

        drawMeterBlend(_meter_x, _meter_y);

        _icon_on->draw(_icon_x, _icon_y);

    } else {

        _meter_off->draw(_meter_x, _meter_y);
        _icon_off->draw(_icon_x, _icon_y);

        if (isBlocked) _icon_bar->draw(_icon_x, _icon_y);

    }

    glPopMatrix();

    _viewFBO.end();

}

void BaseMeterView::drawMeterMask(float level) {

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
    ofRect(0, 0, _maskFBO.getWidth(), floor(level*16.0f) * 41.0f);
    //ofNoFill();

    glPopMatrix();
    _maskFBO.end();

}

void BaseMeterView::drawMeterBlend(float x, float y) {

    /********************************************************
     *    Use a shader to mask the meter_on/off png         *
     *    with the mask rect drawn in the FBO above         *
     ********************************************************/

    _shader.begin();
    glPushMatrix();

	_shader.setTexture("textures[0]", _maskTex, 10);
	_shader.setTexture("textures[1]", *_meter_on, 11);
	_shader.setTexture("textures[2]", *_meter_off, 12);

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
