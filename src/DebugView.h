/*
 *  DebugView.h
 *  Jungle
 *
 *  Created by ollie on 30/06/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */


#ifndef _H_DEBUGVIEW
#define _H_DEBUGVIEW

#include "BaseView.h"
#include "ofxAlphaVideoPlayer.h"
#include "AppDataTypes.h"

class DebugView : public BaseView {
	
public:
	
	DebugView(float width, float height);	//ctor
	//SceneView();						//dtor
	
	void update();
	void draw();

private:
	
protected:
	
};

#endif