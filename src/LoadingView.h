/*
 *  LoadingView.h
 *  Jungle
 *
 *  Created by ollie on 15/07/11.
 *  Copyright 2011 Oliver Marriott. All rights reserved.
 *
 */

#ifndef _H_LOADINGVIEW
#define _H_LOADINGVIEW

#include "BaseView.h"

class LoadingView : public BaseView {

public:

	LoadingView(float width, float height);	//ctor
	//SceneView();							//dtor

	void update();
	//void draw();

private:

protected:

};

#endif
