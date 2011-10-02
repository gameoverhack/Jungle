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
#include "ofxOpenCv.h"
//#include "ofxCvHaarFinder.h"

//#ifdef TARGET_OSX
//#include "ofxQTKitVideoGrabber.h"	// better quality allows recording but OSX only....
//#else
#include "goVideoGrabber.h"			// X-platform and will work with multiple cams but not with ManyCam for some reason...
#include <map>
//#endif

#include "ofxCv.h"
using namespace ofxCv;
using namespace cv;
#include "ofxFaceTracker.h"

//using namespace FACETRACKER;

static int _instanceCount = 0;

class CamController : public BaseState, public ofThread {

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

	int			getInstanceID();
	void		setInstanceID(int instanceID);
    void        setCameraAttributes(PosRotScale prs);

	ofTexture	* getCamTextureRef();

    ofxCvHaarFinder         _finder;

    bool                    _doFaceDetection;
    bool                    _doFaceTracking;

    ofxFaceTracker          _tracker;

private:

    void threadedFunction();

//#ifdef TARGET_OSX
//	ofxQTKitVideoGrabber	_cam;			// this is not X-platform but of/goVideoPlayer does not play well with ManyCam
//#else
    goVideoGrabber	        _cam;
//#endif
	int						_instanceID;

	int                     _width;
	int                     _height;

	ofxCvGrayscaleImage     _greyImage;
    ofxCvColorImage         _colourImage;
    ofxCvColorImage         _camImage;

};


#endif
