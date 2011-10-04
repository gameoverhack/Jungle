#include "BaseMeterView.h"

BaseMeterView::BaseMeterView(float width, float height) : BaseView(width, height) {

    LOG_NOTICE("Constructing BaseMeterView");

    // set up shader
	string vertPath = boost::any_cast<string>(_appModel->getProperty("shaderLevelVertPath"));
	string fragPath = boost::any_cast<string>(_appModel->getProperty("shaderLevelFragPath"));
#if OF_VERSION < 7
	_shader.setup(ofToDataPath(vertPath), ofToDataPath(fragPath));
#else
    _shader.load(ofToDataPath(vertPath), ofToDataPath(fragPath));
#endif
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

    int   totalNumSequences     = _appModel->getCurrentScene()->getNumOfSequences();
    int   thisNumSequence       = _appModel->getCurrentSequence()->getNumber();

    /********************************************************
     *      Draw the Meter to the ViewFBO                   *
     ********************************************************/
    drawMeterMask(_scaledInputLevel, _meterSteps, _meterPixelsForStep, &_meterMaskFBO);
    if (interactionType == kINTERACTION_ATTACKER) drawMeterMask(thisNumSequence - 1, _stationSteps, _stationPixelsForStep, &_stationMaskFBO);

    _viewFBO.begin();

    glPushMatrix();
    ofEnableAlphaBlending();

    glClearColor(0.0, 0.0, 0.0, 1.0); // black background no transparency
    glClear(GL_COLOR_BUFFER_BIT);

    switch(interactionType) {
        case kINTERACTION_ATTACKER:
        {
            _bird->draw(_bird_x, _bird_y);
#if OF_VERSION < 7
            drawMeterShader(_stations_x, _stations_y, &_stationMaskTex, _stations_on, _stations_off);
#else
            drawMeterShader(_stations_x, _stations_y, &_stationMaskFBO.getTextureReference(), _stations_on, _stations_off);
#endif
            _button_off->draw(_button_x, _button_y);

            if (isInteractive && _scaledInputLevel > 0.05f) {
                _button_on->draw(_button_x, _button_y);
            } else if (!isInteractive && _scaledInputLevel > 0.05f) {
                _button_off->draw(_button_x, _button_y);
                _button_deny->draw(_button_x, _button_y);
            }

            break;
        }
        case kINTERACTION_VICTIM:
        {
            int sceneCurrentFrame   = _appModel->getCurrentSceneFrame();
            int sceneTotalFrames    = _appModel->getCurrentSceneNumFrames();
            float turtlePosY        = _turtle_bar->getHeight() * ((float)sceneCurrentFrame/(float)sceneTotalFrames);

            _turtle_bar->draw(_turtle_bar_x, _turtle_bar_y);
            _turtle->draw(_turtle_x, _turtle_y - turtlePosY);
            _top_off->draw(_top_x, _top_y);

            if (isInteractive && _scaledInputLevel > 0.99f) {
                _top_on->draw(_top_x, _top_y);
            } else if (!isInteractive && _scaledInputLevel > 0.05f) {
                _top_off->draw(_top_x, _top_y);
                _top_deny->draw(_top_x, _top_y);
            }

            break;
        }
    }

    if (isInteractive) {
#if OF_VERSION < 7
        drawMeterShader(_meter_x, _meter_y, &_meterMaskTex, _meter_on, _meter_off);
#else
        drawMeterShader(_meter_x, _meter_y, &_meterMaskFBO.getTextureReference(), _meter_on, _meter_off);
#endif
    } else {

        _meter_off->draw(_meter_x, _meter_y);

    }

    ofDisableAlphaBlending();
    glPopMatrix();

    _viewFBO.end();

}
#if OF_VERSION < 7
void BaseMeterView::drawMeterMask(float input, int meterSteps, float meterPixelsForStep, ofxFbo * maskFBO) {
#else
void BaseMeterView::drawMeterMask(float input, int meterSteps, float meterPixelsForStep, ofFbo * maskFBO) {
#endif
    /********************************************************
     *    Draw a rect in an FBO to mask the level meter     *
     ********************************************************/

    maskFBO->begin();
    glPushMatrix();

    glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
    glClear(GL_COLOR_BUFFER_BIT);

	ofSetColor(255, 255, 255, 255); // no tinting
	ofFill();
	glTranslatef(0.0f, maskFBO->getHeight(), 0.0f);
	glScalef(1.0f, -1.0f, 1.0f);
    ofRect(0, 0, maskFBO->getWidth(), floor(input * meterSteps) * meterPixelsForStep);
    //ofNoFill();

    glPopMatrix();
    maskFBO->end();

}
#if OF_VERSION < 7
void BaseMeterView::drawMeterMask(int input, int meterSteps, float meterPixelsForStep, ofxFbo * maskFBO) {
#else
void BaseMeterView::drawMeterMask(int input, int meterSteps, float meterPixelsForStep, ofFbo * maskFBO) {
#endif
    /********************************************************
     *    Draw a rect in an FBO to mask the level meter     *
     ********************************************************/

    maskFBO->begin();
    glPushMatrix();

    glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
    glClear(GL_COLOR_BUFFER_BIT);

	ofSetColor(255, 255, 255, 255); // no tinting
	ofFill();
	glTranslatef(0.0f, maskFBO->getHeight(), 0.0f);
	glScalef(1.0f, -1.0f, 1.0f);
    ofRect(0, 0, maskFBO->getWidth(), input * meterPixelsForStep);
    //ofNoFill();

    glPopMatrix();
    maskFBO->end();

}

void BaseMeterView::drawMeterShader(float x, float y, ofTexture *maskTex, ofTexture *meterOnTex, ofTexture *meterOffTex) {

    /********************************************************
     *    Use a shader to mask the meter_on/off png         *
     *    with the mask rect drawn in the FBO above         *
     ********************************************************/

    _shader.begin();
    glPushMatrix();
#if OF_VERSION < 7
	_shader.setTexture("textures[0]", *maskTex, 10);
	_shader.setTexture("textures[1]", *meterOnTex, 11);
	_shader.setTexture("textures[2]", *meterOffTex, 12);
#else
	_shader.setUniformTexture("textures[0]", *maskTex, 10);
	_shader.setUniformTexture("textures[1]", *meterOnTex, 11);
	_shader.setUniformTexture("textures[2]", *meterOffTex, 12);
#endif

    _shader.setUniform1f("level", 1.0f);

    glTranslatef(x, y, 1.0f);

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 0);	glVertex2f(0, 0);
	glTexCoord2f(0, maskTex->getHeight()); glVertex2f(0, maskTex->getHeight());
	glTexCoord2f(maskTex->getWidth(), 0); glVertex2f(maskTex->getWidth(), 0);
	glTexCoord2f(maskTex->getWidth(), maskTex->getHeight()); glVertex2f(maskTex->getWidth(), maskTex->getHeight());
	glEnd();

	glPopMatrix();
	_shader.end();

}
