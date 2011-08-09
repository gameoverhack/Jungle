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
	// Set up cameras

	/* temp use static images for cam
	ofImage frameImage;
	assert(frameImage.loadImage("gueule.jpg")); // nasty but i keep wondering what's wrong and i just haven't copied the file ;-)
	
	_cameraOne.allocate(frameImage.getWidth(), frameImage.getHeight(), GL_RGB);
	_cameraOne.loadData(frameImage.getPixels(), // Load pixels in to texture
						frameImage.getWidth(), 
						frameImage.getHeight(), 
						GL_RGB);
	
	_cameraTwo.allocate(frameImage.getWidth(), frameImage.getHeight(), GL_RGB);
	_cameraTwo.loadData(frameImage.getPixels(), // Load pixels in to texture
						frameImage.getWidth(), 
						frameImage.getHeight(), 
						GL_RGB);
	*/
	// set up fbos
	
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
	
	// set up shader
	string vertPath = boost::any_cast<string>(_appModel->getProperty("shaderVertPath"));
	string fragPath = boost::any_cast<string>(_appModel->getProperty("shaderFragPath"));
	
	_shader.setup(ofToDataPath(vertPath), ofToDataPath(fragPath));
	
}

void SceneView::update() {
	
	goThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer(); //getCurrentSequence()->getMovie();
	ofTexture		* sceneTexture;
	CamTransform	* actorTransform;

	// get the video texture
	//currentMovie->draw(0, 0);
	sceneTexture = &(currentMovie->getTextureReference());
	currentFrame = _appModel->getCurrentFrame(); //currentMovie->getCurrentFrame();
	
	glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
	ofSetColor(255,255,255); // no tinting

	if(_appModel->getCurrentIsFrameNew()){
		// draw characters faces to new positions
		// TODO: This needs to be smarter
		drawCharacter(&_vic1FBO, _appModel->getVictimCamTexRef(), &(_appModel->getCurrentSequence()->getTransformVector("vic1")->at(currentFrame)));
		drawCharacter(&_atk1FBO, _appModel->getAttackCamTexRef(), &(_appModel->getCurrentSequence()->getTransformVector("atk1")->at(currentFrame)));
		
		if (_appModel->getCurrentSequence()->getTransformCount() > 2) {
			drawCharacter(&_atk2FBO, _appModel->getAttackCamTexRef(), &(_appModel->getCurrentSequence()->getTransformVector("atk2")->at(currentFrame)));
		}
	}
	
	_viewFBO.begin();
	
	// set up shader stuff
	_shader.begin();
	_shader.setTexture("textures[0]", *sceneTexture, 10);
	_shader.setTexture("textures[1]", _vic1Tex, 11);
	_shader.setTexture("textures[2]", _atk1Tex, 12);
	
	int numTextures = 3;
	if (_appModel->getCurrentSequence()->getTransformCount() > 2) {
		_shader.setTexture("textures[3]", _atk2Tex, 13);
		numTextures++;
	}
	_shader.setUniform1i("numTextures", numTextures);
	_shader.setUniform1i("showUnmaskedTextures", (int)(boost::any_cast<bool>(_appModel->getProperty("showUnmaskedTextures"))));
	_shader.setUniform1f("blendRatio", boost::any_cast<float>(_appModel->getProperty("shaderBlendRatio")));
	_shader.setUniform1f("gammaCorrection", boost::any_cast<float>(_appModel->getProperty("shaderGammaCorrection")));
	
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 0);	glVertex2f(0, 0);
	glTexCoord2f(0, _viewHeight); glVertex2f(0, _viewHeight);
	glTexCoord2f(_viewWidth, 0); glVertex2f(_viewWidth, 0);
	glTexCoord2f(_viewWidth, _viewHeight); glVertex2f(_viewWidth, _viewHeight);
	glEnd();
	
	_shader.end();
	_viewFBO.end();
	
}

void SceneView::drawCharacter(ofxFbo * targetFBO, 
							  ofTexture * faceTexture, 
							  CamTransform *transform) {
	// set up draw state
	targetFBO->begin();
	glPushMatrix();

	glClear(GL_COLOR_BUFFER_BIT); // clear frame
	
	// do the transform
	
	// translate to the right place
	glTranslatef(transform->x, transform->y, 0.0);

	// magic transform shenanigans -> move to flash analyze functions eventually
	float rot = transform->rotation;
	float sclX = transform->w/640.0f;
	float sclY = sclX;
	
	if (transform->rotation < -90.0 || transform->rotation > 90.0) {
		sclY = -sclY;
		rot = -rot;
	}

	// flip the camera requires scale and rot to be negativised...
	//glScalef(-1.0f, 1.0f, 1.0f);
	//rot = -rot;
	
	glScalef(sclX, sclY, 1.0f);
	
	// rotate the head
	glRotatef(rot, 0.0f, 0.0f, 1.0f);
	
	// translate back half (/4 due to glScale) the width/height of the camera texture
	glTranslatef(-(faceTexture->getWidth()/4.0), -(faceTexture->getHeight()/4.0), 0.0);
	
	// scale down camera texture
	glScalef(0.5, 0.5, 0);
	
	// draw face texture 
	faceTexture->draw(0,0);
	
	// end rendering 
	glPopMatrix();
	targetFBO->end();	
}