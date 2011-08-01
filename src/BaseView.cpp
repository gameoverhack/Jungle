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
	
	// Create black pixels for inital texture, TODO: Potentially not required
	//unsigned char * black = (unsigned char *)calloc(_viewWidth * _viewHeight * 4, sizeof(unsigned char));
	
	// allocate the fbo texture
	//_viewFBOTexture.allocate(_viewWidth, _viewHeight, GL_RGBA);
	//_viewFBOTexture.loadData(black, _viewWidth, _viewHeight, GL_RGBA);
	
	// set up fbo and attach texture
	_viewFBO.allocate(_viewWidth, _viewHeight);
	//_viewFBO.attach(_viewFBOTexture);
	
	// Free black pixels
	//free(black);
}

BaseView::~BaseView() {
	//_viewFBO.destroy();
	//_viewFBOTexture.clear();
}

// Returns the views fbo 
ofFbo * BaseView::getViewFBO(){
	return &_viewFBO;
}

ofTexture * BaseView::getViewFBOTexture(){
	return &_viewFBO.getTextureReference();
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