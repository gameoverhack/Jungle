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
