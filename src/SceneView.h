/*
 *  SceneView.h
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_SCENEVIEW
#define _H_SCENEVIEW

#include "BaseView.h"
#include "ofxShader.h"

class SceneView : public BaseView {
	
public:
	
	SceneView(float width, float height);	//ctor
	//SceneView();							//dtor
	
	void update();
	//void draw();
	
	void drawCharacter(ofxFbo * targetFBO, 
					   ofTexture * faceTexture, 
					   CamTransform *transform);
	
private:
	
	//Scene * _currentScene;
	ofTexture _cameraOne;
	ofTexture _cameraTwo;
	
	// fbo stuff
	ofTexture _vic1Tex, _atk1Tex, _atk2Tex;
	ofxFbo _vicFBO, _atk1FBO, _atk2FBO;
	
	// Shader stuff
	ofxShader _shader;
	
protected:
	
};

#endif