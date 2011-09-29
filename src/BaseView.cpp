/*
 *  BaseView.cpp
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "BaseView.h"

BaseView::BaseView(float width, float height) {
	// save parameters
	_viewWidth = width;
	_viewHeight = height;

#if OF_VERSION < 7
	// Create black pixels for inital texture, TODO: Potentially not required
	unsigned char * black = (unsigned char *)calloc(_viewWidth * _viewHeight * 4, sizeof(unsigned char));

	// allocate the fbo texture
	_viewFBOTexture.allocate(_viewWidth, _viewHeight, GL_RGBA);
	_viewFBOTexture.loadData(black, _viewWidth, _viewHeight, GL_RGBA);

	// set up fbo and attach texture
	_viewFBO.setup(_viewWidth, _viewHeight);
	_viewFBO.attach(_viewFBOTexture);

    // Free black pixels
	free(black);
#else
    _viewFBO.allocate(_viewWidth, _viewHeight);
#endif

}

BaseView::~BaseView() {
#if OF_VERSION < 7
	_viewFBO.detach(0);
	_viewFBOTexture.clear();
#endif
}

// Returns the views fbo
#if OF_VERSION < 7
ofxFbo * BaseView::getViewFBO() {
#else
ofFbo * BaseView::getViewFBO() {
#endif
	return &_viewFBO;
}

ofTexture * BaseView::getViewFBOTexture() {
#if OF_VERSION < 7
	return &_viewFBOTexture;
#else
    return &_viewFBO.getTextureReference();
#endif
}

float BaseView::getWidth() {
    return _viewWidth;
}
float BaseView::getHeight() {
    return _viewHeight;
}

void BaseView::draw() {
	glPushMatrix();
	_viewFBO.draw(0, 0);
	glPopMatrix();
}

void BaseView::draw(float x, float y) {
	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	draw();
	glPopMatrix();
}

void BaseView::draw(float x, float y, float width, float height) {
	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	glScalef(width/_viewWidth, height/_viewHeight, 0.0f);
	draw();
	glPopMatrix();
}
