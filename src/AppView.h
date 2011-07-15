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
#include "LoadingView.h"
#include "DebugView.h"

class AppView : public BaseView {

public:
	
	AppView(float width, float height);		//ctor
	//~AppView();							//dtor
	
	void update();
	void draw();
	
private:
	LoadingView *_loadingView;
	SceneView *_sceneView;
	DebugView *_debugView;
	
protected:
	
};

#endif