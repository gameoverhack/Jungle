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
#include "AttackView.h"
#include "VictimView.h"

class AppView : public BaseView {

public:

	AppView(float width, float height);		//ctor
	~AppView();							//dtor

	void update();
	void draw();

private:

	LoadingView * _loadingView;
	SceneView	* _sceneView;
	DebugView	* _debugView;
	AttackView  * _attackView;
	VictimView  * _victimView;

    float         _fade;

protected:

};

#endif
