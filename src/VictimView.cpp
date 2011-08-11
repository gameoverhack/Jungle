#include "VictimView.h"

VictimView::VictimView(float width, float height) : BaseView(width, height) {

	LOG_NOTICE("Setting up VictimView");

    // allocate fbo's
    _maskTex.allocate(124, 672, GL_RGBA); // size of the meter_on/off png
    _maskFBO.setup(124, 672);
    _maskFBO.attach(_maskTex);

    // set up shader
	string vertPath = boost::any_cast<string>(_appModel->getProperty("shaderLevelVertPath"));
	string fragPath = boost::any_cast<string>(_appModel->getProperty("shaderLevelFragPath"));

	_shader.setup(ofToDataPath(vertPath), ofToDataPath(fragPath));

}

void VictimView::update() {

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    ofTexture   * meter_on      = _appModel->getGraphicTex(kGFX_METER_ON);
    ofTexture   * meter_off     = _appModel->getGraphicTex(kGFX_METER_OFF);
    ofTexture   * meter_level   = _appModel->getGraphicTex(kGFX_METER_LEVEL);
    ofTexture   * icon_on       = _appModel->getGraphicTex(kGFX_SCREAM_ON);
    ofTexture   * icon_off      = _appModel->getGraphicTex(kGFX_SCREAM_OFF);
    ofTexture   * icon_bar      = _appModel->getGraphicTex(kGFX_SCREAM_BAR);

    int currentInteractivity    = _appModel->getCurrentInteractivity();
    bool isInteractive          = (currentInteractivity == kINTERACTION_BOTH || currentInteractivity == kINTERACTION_VICTIM);

    float scaledInputLevel      = (float)_appModel->getARDRawPins()[0]/250.0f;      // make more complex soon ;-)
    bool blockInput             = ((float)_appModel->getARDRawPins()[0] > 10.0f);   // make more complex soon ;-)

    float icon_x                = 5.0f;
    float icon_y                = 700.0f;
    float meter_x               = 5.0f;
    float meter_y               = 10.0f;

    if (isInteractive) drawMeterMask(scaledInputLevel);

    _viewFBO.begin();
    glPushMatrix();

    glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
    glClear(GL_COLOR_BUFFER_BIT);

    ofSetColor(255, 255, 255, 255); // no tinting

    if (isInteractive) {

        drawMeterBlend(meter_x, meter_y, meter_on, meter_off);

        icon_on->draw(icon_x, icon_y);

    } else {

        meter_off->draw(meter_x, meter_y);
        icon_off->draw(icon_x, icon_y);

        if (blockInput) icon_bar->draw(icon_x, icon_y);

    }

    glPopMatrix();

    _viewFBO.end();

}

void VictimView::drawMeterMask(float level) {

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
    ofRect(0, 0, _maskFBO.getWidth(), floor(level*_maskFBO.getHeight()/42.0f) * 42.0f);
    //ofNoFill();

    glPopMatrix();
    _maskFBO.end();
}

void VictimView::drawMeterBlend(float x, float y, ofTexture * meter_on, ofTexture * meter_off) {

    /********************************************************
     *    Use a shader to mask the meter_on/off png         *
     *    with the mask rect drawn in the FBO above         *
     ********************************************************/

    _shader.begin();
    glPushMatrix();

	_shader.setTexture("textures[0]", _maskTex, 10);
	_shader.setTexture("textures[1]", *meter_on, 11);
	_shader.setTexture("textures[2]", *meter_off, 12);

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