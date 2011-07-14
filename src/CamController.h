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

#include "AppModel.h"
#include "Logger.h"

//#include "goVideoGrabber.h"			// X-platform and will work with multiple cams but not with ManyCam for some reason...
#include "ofxQTKitVideoGrabber.h"	// better quality allows recording but OSX only....

static int _instanceCount = 0;

class CamController {
	
public:
	
	CamController();
	~CamController();
	
	bool		setup(int deviceID, int x, int y);
	bool		setup(string deviceID, int x, int y);
	
	void		update();
	
	int			getInstanceID();
	void		setInstanceID(int instanceID);
	
	ofTexture	* getCamTextureRef();
	
private:

	ofxQTKitVideoGrabber	_cam;			// this is not X-platform but of/goVideoPlayer does not play well with ManyCam
	int						_instanceID;
	
};


#endif