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
#include "goVideoPlayer.h"
#include "AppDataTypes.h"

class DebugView : public BaseView {

public:

	DebugView(float width, float height);	//ctor
	//SceneView();						//dtor

	void update();

private:

    void plot(float* array, int length, float scale, float offset);
	void plotDraw(float* array, int width, int height, int binSize, float scaleW = 1.0f, float scaleH = 1.0f);

};

#endif
