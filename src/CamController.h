/*
 *  CamController.h
 *  Jungle
 *
 *  Created by gameover on 7/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_CAMCONTROLLER
#define _H_CAMCONTROLLER

#include "BaseState.h"
#include "AppModel.h"
#include "Logger.h"

#ifdef TARGET_OSX
#include "ofxQTKitVideoGrabber.h"	// better quality allows recording but OSX only....
#else
#include "goVideoGrabber.h"			// X-platform and will work with multiple cams but not with ManyCam for some reason...
#endif

static int _instanceCount = 0;

class CamController : public BaseState {

public:

	CamController();
	~CamController();

	bool		setup(int deviceID, int x, int y);
#ifdef TARGET_OSX
	bool		setup(string deviceID, int x, int y);
#endif

	void		update();

	int			getInstanceID();
	void		setInstanceID(int instanceID);

	ofTexture	* getCamTextureRef();

private:

#ifdef TARGET_OSX
	ofxQTKitVideoGrabber	_cam;			// this is not X-platform but of/goVideoPlayer does not play well with ManyCam
#else
    goVideoGrabber	        _cam;
#endif
	int						_instanceID;

};


#endif
