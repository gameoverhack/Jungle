/*
 *  BaseView.h
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_BASEVIEW
#define _H_BASEVIEW

#include "AppModel.h"
#include "ofxFbo.h"

class BaseView {

public:

	BaseView(float width, float height);	//ctor
	virtual ~BaseView();						//dtor

	virtual void update() = 0; // update view fbo to draw image

	virtual void draw(); // draw out the view fbo
	virtual void draw(float x, float y);
	virtual void draw(float x, float y, float width, float height);

	// Returns the views fbo
	ofxFbo		* getViewFBO();
	ofTexture	* getViewFBOTexture();

private:

protected:

	float		_viewHeight, _viewWidth;

	ofxFbo		_viewFBO; // final output FBO

	ofTexture	_viewFBOTexture; // Texture for final output FBO

};

#endif
