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
#if OF_VERSION < 7
#include "ofxShader.h"
#endif

class SceneView : public BaseView {

public:

	SceneView(float width, float height);	//ctor
	//SceneView();							//dtor

	void update();
	//void draw();
#if OF_VERSION < 7
	void drawCharacter(ofxFbo * targetFBO,
					   ofTexture * faceTexture,
					   CamTransform *transform,
					   PosRotScale * prs, bool flipMe);
#else
	void drawCharacter(ofFbo * targetFBO,
					   ofTexture * faceTexture,
					   CamTransform *transform,
					   PosRotScale * prs, bool flipMe);
#endif
private:

    int currentFrame;
	//Scene * _currentScene;
	ofTexture _cameraOne;
	ofTexture _cameraTwo;

	// fbo stuff
#if OF_VERSION < 7
	ofTexture _vic1Tex, _atk1Tex, _atk2Tex;
	ofxFbo _vic1FBO, _atk1FBO, _atk2FBO;
#else
    ofFbo _vic1FBO, _atk1FBO, _atk2FBO;
#endif
	float   currentVideoHeight, currentVideoWidth;

	// Shader stuff
#if OF_VERSION < 7
	ofxShader _shader;
#else
    ofShader  _shader;
#endif

protected:

};

#endif
