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

//#define USE_DUMMY

#include "BaseState.h"
#include "AppModel.h"
#include "Logger.h"
#include "SerializationUtils.h"

#ifdef TARGET_OSX
#include "ofxQTKitVideoGrabber.h"	// better quality allows recording but OSX only....
#else
#include "goVideoGrabber.h"			// X-platform and will work with multiple cams but not with ManyCam for some reason...
#include <map>
#endif

static int _instanceCount = 0;

class CamController : public BaseState {

public:

	CamController();
	~CamController();

    bool		setup(int deviceID, int w, int h);

    void        loadAttributes();
    void        saveAttributes();

#ifdef TARGET_WIN32
	void        showVideoSettings();
	void        loadSettings();
	void        saveSettings();
#else
	bool		setup(string deviceID, int w, int h);
#endif

	void		update();
    void        drawDebug(float x, float y, float width, float height);

	int			getInstanceID();
	void		setInstanceID(int instanceID);
    void        setCameraAttributes(PosRotScale * prs);
    void        setFakeAttributes(PosRotScale * prs);

	ofTexture* getCamTextureRef();

    void mouseMoved(ofMouseEventArgs &e);
	void mouseDragged(ofMouseEventArgs &e);
	void mousePressed(ofMouseEventArgs &e);
	void mouseReleased(ofMouseEventArgs &e);

private:

#ifdef TARGET_OSX
	ofxQTKitVideoGrabber	_cam;			// this is not X-platform but of/goVideoPlayer does not play well with ManyCam
#else
#ifdef USE_DUMMY
    ofVideoPlayer           _cam;
#else
    goVideoGrabber	        _cam;
#endif
#endif
	int						_instanceID;

	int                     _width;
	int                     _height;

    bool                    _isCamInit;

};

#endif
