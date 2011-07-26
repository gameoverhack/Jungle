/*
 *  AppController.h
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_APPCONTROLLER
#define _H_APPCONTROLLER

#include "ofMain.h"
#include "AppView.h"
#include "AppModel.h"

#include "DataController.h"
#include "CamController.h"
#include "Constants.h"

class AppController : public ofBaseApp {
	
public:
	
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	void swapCameras();
	
	void setWindowed();
	void setFullscreen();
	void toggleFullscreen();
	
private:
	
	// state
	AppControllerState	_state;
	string				_stateMessage;
	
	// data controller stuff
	DataController	*_dataController;
	
	// View stuff
	AppView			*_appView;
	
	CamController	*_camControllers[2];
	
	bool			isFullScreen;
	bool			loadNextMovies;
	
protected:
	
};

#endif