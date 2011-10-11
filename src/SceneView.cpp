/*
 *  SceneView.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "SceneView.h"

SceneView::SceneView(float width, float height) : BaseView(width ,height) {

    LOG_NOTICE("Setting up SceneView");

    /********************************************************
     *              Allocate Textures and FBO's           	*
     ********************************************************/
#if OF_VERSION < 7
	// Allocate texture and attach*/
	_vic1Tex.allocate(_viewWidth, _viewHeight, GL_RGBA);
	_vic1FBO.setup(_viewWidth, _viewHeight);
	_vic1FBO.attach(_vic1Tex);

	_atk1Tex.allocate(_viewWidth, _viewHeight, GL_RGBA);
	_atk1FBO.setup(_viewWidth, _viewHeight);
	_atk1FBO.attach(_atk1Tex);

	_atk2Tex.allocate(_viewWidth, _viewHeight, GL_RGBA);
	_atk2FBO.setup(_viewWidth, _viewHeight);
	_atk2FBO.attach(_atk2Tex);
#else
    _vic1FBO.allocate(_viewWidth, _viewHeight);
    _atk1FBO.allocate(_viewWidth, _viewHeight);
    _atk2FBO.allocate(_viewWidth, _viewHeight);
#endif

#ifndef OPTIMISED
	// if not optimised, load unoptimised shader.
	// set up shader
	string vertPath = boost::any_cast<string>(_appModel->getProperty("shaderHeadVertPath"));
	string fragPath = boost::any_cast<string>(_appModel->getProperty("shaderHeadFragPath"));
#else
	// else load up optimised shader with hard coded values
	// TODO_OPTIMISED
	string vertPath = boost::any_cast<string>(_appModel->getProperty("shaderOptimisedHeadVertPath"));
	string fragPath = boost::any_cast<string>(_appModel->getProperty("shaderOptimisedHeadFragPath"));
#endif
	
#if OF_VERSION < 7
	_shader.setup(ofToDataPath(vertPath), ofToDataPath(fragPath));
#else
    _shader.load(ofToDataPath(vertPath), ofToDataPath(fragPath));
#endif
}

void SceneView::update() {

    /********************************************************
     *          Get references needed to do drawing      	*
     ********************************************************/

	goThreadedVideo * currentMovie  = _appModel->getCurrentVideoPlayer(); //getCurrentSequence()->getMovie();
	ofTexture		* sceneTexture;
	CamTransform	* actorTransform;

	// get the video texture
	sceneTexture = &(currentMovie->getTextureReference());

	currentVideoWidth  = sceneTexture->getWidth();
	currentVideoHeight = sceneTexture->getHeight();

	currentFrame = _appModel->getCurrentSequenceFrame(); //currentMovie->getCurrentFrame();

	glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
	glClear(GL_COLOR_BUFFER_BIT);

	ofSetColor(255,255,255); // no tinting

    /********************************************************
     *          Draw each head/webcam into an FBO       	*
     ********************************************************/

	if(true){

		ofTexture * victimTex;
		ofTexture * attackTex;
        PosRotScale * victimPRS;
        PosRotScale * attackPRS;

		if (_appModel->getFacePresent(0)) {
            victimTex = _appModel->getVictimCamTexRef();
            victimPRS = _appModel->getCameraAttributes(0);
		} else {
		    victimTex = _appModel->getFakeVictimCamTexRef();
		    victimPRS = _appModel->getFakeAttributes(0);
		}

        if (_appModel->getFacePresent(1)) {
            attackTex = _appModel->getAttackCamTexRef();
            attackPRS = _appModel->getCameraAttributes(1);
		} else {
		    attackTex = _appModel->getFakeAttackCamTexRef();
		    attackPRS = _appModel->getFakeAttributes(1);
		}

		drawCharacter(&_vic1FBO, victimTex, &(_appModel->getCurrentSequence()->getTransformVector("vic1")->at(currentFrame)), victimPRS);
		drawCharacter(&_atk1FBO, attackTex, &(_appModel->getCurrentSequence()->getTransformVector("atk1")->at(currentFrame)), attackPRS);

		if (_appModel->getCurrentSequence()->getTransformCount() > 2) {
			drawCharacter(&_atk2FBO, attackTex, &(_appModel->getCurrentSequence()->getTransformVector("atk2")->at(currentFrame)), attackPRS);
		}
	}

    /********************************************************
     *      Draw Shader (composites webcams and movie)     	*
     ********************************************************/

	_viewFBO.begin();

	// set up shader stuff
	_shader.begin();
#if OF_VERSION < 7
	_shader.setTexture("textures[0]", *sceneTexture, 10);
	_shader.setTexture("textures[1]", _vic1Tex, 11);
	_shader.setTexture("textures[2]", _atk1Tex, 12);
#else
	_shader.setUniformTexture("textures[0]", *sceneTexture, 10);
	_shader.setUniformTexture("textures[1]", _vic1FBO.getTextureReference(), 11);
	_shader.setUniformTexture("textures[2]", _atk1FBO.getTextureReference(), 12);
#endif

	int numTextures = 3;
	if (_appModel->getCurrentSequence()->getTransformCount() > 2) {
#if OF_VERSION < 7
		_shader.setTexture("textures[3]", _atk2Tex, 13);
#else
        _shader.setUniformTexture("textures[3]", _atk2FBO.getTextureReference(), 13);
#endif
		numTextures++;
	}
	
	// set number of heads we're blending.
	_shader.setUniform1i("numTextures", numTextures);
	_shader.setUniform1i("showUnmaskedTextures", (int)boost::any_cast<bool>(_appModel->getProperty("showUnmaskedTextures")));
	
#ifndef OPTIMISED
	// if not optimised, load stuff from properties, else it has been hard coded into optimised shader
	_shader.setUniform1f("blendRatio", boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio")));
	_shader.setUniform1f("gammaCorrection", boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection")));
#else
	// shader values are hard coded
#endif
	
    glPushMatrix();

    // scale to screen
    glScalef(_viewWidth/currentVideoWidth, _viewHeight/currentVideoHeight, 0.0f);

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 0);	glVertex2f(0, 0);
	glTexCoord2f(0, currentVideoHeight); glVertex2f(0, currentVideoHeight);
	glTexCoord2f(currentVideoWidth, 0); glVertex2f(currentVideoWidth, 0);
	glTexCoord2f(currentVideoWidth, currentVideoHeight); glVertex2f(currentVideoWidth, currentVideoHeight);
	glEnd();

    glPopMatrix();

	_shader.end();
	_viewFBO.end();

}
#if OF_VERSION < 7
void SceneView::drawCharacter(ofxFbo * targetFBO,
							  ofTexture * faceTexture,
							  CamTransform *transform,
							  PosRotScale * prs) {
#else
void SceneView::drawCharacter(ofFbo * targetFBO,
							  ofTexture * faceTexture,
							  CamTransform *transform,
							  PosRotScale * prs) {
#endif

    float width  = faceTexture->getWidth();
    float height = faceTexture->getHeight();

	// set up draw state
	targetFBO->begin();
	glPushMatrix();

	glClear(GL_COLOR_BUFFER_BIT); // clear frame
    ofEnableAlphaBlending();
    /********************************************************
     *      Magic numbers on the Transform data         	*
     *      should be moved to the Analyzer eventually    	*
     ********************************************************/

	// magic transform shenanigans -> move to flash analyze functions eventually
	float rot = transform->rotation;

	float sclX;
	
#ifndef OPTIMISED
	// if not optimised, get scale method
	int scaleMethod = boost::any_cast<int>(_appModel->getProperty("tryScaleMethod"));
#else
	int scaleMethod = 1; // TODO_OPTIMISED
#endif
	if (scaleMethod == 0) {
        sclX = ABS(transform->scaleX);
	} else {
        sclX = transform->w/400.0f;
	}

	float sclY = sclX;

	if (transform->rotation < -90.0 || transform->rotation > 90.0) {
		sclY = -sclY;
		rot = -rot;
	}

    /********************************************************
     *     Translate, Scale, Rotate webcam by Transform   	*
     ********************************************************/

    // transform translation
	glTranslatef(transform->x, transform->y, 0.0);

    // transform scale
	glScalef(sclX, sclY, 1.0f);

	// transform rotation
	glRotatef(rot, 0.0f, 0.0f, 1.0f);

	// translate back half (/4 due to glScale) the width/height of the camera texture
	glTranslatef(-(width/4.0), -(height/4.0), 0.0);

    // can we do like this?
    /*glTranslatef(width/2.0f + transform->x, height/2.0f + transform->y, 0.0f);
    glScalef(sclX, sclY, 1.0f);
    glRotatef(rot, 0.0f, 0.0f, 1.0f);
    glTranslatef(-width/2.0f - transform->x, -height/2.0f - transform->y, 0.0f);*/

    /********************************************************
     *      Translate, Scale, Rotate webcam by User     	*
     ********************************************************/

	// flip the camera requires scale and rot to be negativised...
	//glScalef(-1.0f, 1.0f, 1.0f);
	//rot = -rot;

    glTranslatef(width/2.0f + prs->x, height/2.0f + prs->y, 0.0f);
    glScalef(prs->s, prs->s, 1.0f);
    glRotatef(prs->r, 0.0f, 0.0f, 1.0f);
    glTranslatef(-width/2.0f - prs->x, -height/2.0f - prs->y, 0.0f);

	// draw face texture
	faceTexture->draw(0,0);

	// end rendering
	ofDisableAlphaBlending();
	glPopMatrix();

	targetFBO->end();
}
