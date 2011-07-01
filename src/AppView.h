/*
 *  AppView.h
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_APPVIEW
#define _H_APPVIEW

#include "BaseView.h"
#include "SceneView.h"

class AppView : public BaseView {

public:
	
	AppView(int width, int height);		//ctor
	//~AppView();						//dtor
	
	void update();
	void draw();
	
private:
	
	SceneView *_sceneView;
	
protected:
	
};

#endif