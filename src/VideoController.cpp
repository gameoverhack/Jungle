/*
 *  VideoController.cpp
 *  Jungle
 *
 *  Created by gameover on 27/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#include "VideoController.h"


VideoController::VideoController() {
	
	LOG_VERBOSE("Constructing VideoController and adding listeners");
	
	/* setup listeners for error and loadDone
	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	goThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer();
	
	ofAddListener(nextMovie->loadDone, this, &VideoController::loaded);
	ofAddListener(nextMovie->error, this, &VideoController::error);
	ofAddListener(currentMovie->loadDone, this, &VideoController::loaded);
	ofAddListener(currentMovie->error, this, &VideoController::error);*/
	
}

VideoController::~VideoController() {
	
	LOG_VERBOSE("Destroying VideoController and removing listeners");
	
	/* remove listeners for error and loadDone
	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	goThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer();
	
	ofRemoveListener(nextMovie->loadDone, this, &VideoController::loaded);
	ofRemoveListener(nextMovie->error, this, &VideoController::error);
	ofRemoveListener(currentMovie->loadDone, this, &VideoController::loaded);
	ofRemoveListener(currentMovie->error, this, &VideoController::error);*/
	
	// close any open movies
	//nextMovie->close();
	//currentMovie->close();
	
}

void VideoController::registerStates() {
	LOG_VERBOSE("Registering States");
	
	registerState(kVIDCONTROLLER_READY, "kVIDCONTROLLER_READY");
	registerState(kVIDCONTROLLER_CURRENTVIDONE, "kVIDCONTROLLER_CURRENTVIDONE");
	registerState(kVIDCONTROLLER_NEXTVIDLOADING, "kVIDCONTROLLER_NEXTVIDLOADING");
	registerState(kVIDCONTROLLER_NEXTVIDREADY, "kVIDCONTROLLER_NEXTVIDREADY");
	registerState(kVIDCONTROLLER_NEXTVIDERROR, "kVIDCONTROLLER_NEXTVIDERROR");
	
	setState(kVIDCONTROLLER_READY);
}

void VideoController::update() {
	
	if (_forceCurrentLoad) forceUpdate();
	
	// update the current currentMovie/videoplayer
	goThreadedVideo * currentMovie		= _appModel->getCurrentVideoPlayer();
	Scene			* currentScene		= _appModel->getCurrentScene();
	Sequence		* currentSequence	= _appModel->getCurrentSequence();
	
	currentMovie->update();
	
	// set global vars for current frame and isFrameNew on the Model -> hopefully eliminates misreadings???
	int	 currentFrame	= currentMovie->getCurrentFrame();
	int	 totalFrames	= currentMovie->getTotalNumFrames();
	bool isFrameNew		= currentMovie->isFrameNew();
	
	_appModel->setCurrentFrame(currentFrame);
	//_appModel->setCurrentFrameTotal(totalFrames); // no need for now?
	_appModel->setCurrentIsFrameNew(isFrameNew);
	
	if (currentFrame > totalFrames - 12 && !_cachedLoopAndState) {
		
		// set loopstate
		if (currentSequence->getInteractivity() == "both") {
			currentMovie->setLoopState(OF_LOOP_NORMAL);
		} else {
			currentMovie->setLoopState(OF_LOOP_NONE);
		}
		_cachedLoopAndState = true;
		
		// 'cache' (ie., half) load loop sequence if we're on an 'a' type movie
		
		string nextSequenceName = currentSequence->getNextSequenceName();
		if (nextSequenceName != "" && nextSequenceName != "__FINAL_SEQUENCE__") {
			loadMovie(currentScene->getSequence(nextSequenceName));
		} else if (nextSequenceName == "__FINAL_SEQUENCE__") {
			// end of SCENE? which seq to cache?
		}

	}
	
	if (currentMovie->getIsMovieDone()) {
		LOG_VERBOSE("HEREEEEEE");
		toggleVideoPlayers();
		setState(kVIDCONTROLLER_CURRENTVIDONE);
	}
	
}

void VideoController::forceUpdate() {
	// used to force loading of a movie if we're not actually updating/drawing it
	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	nextMovie->psuedoUpdate();
	nextMovie->psuedoDraw();
}

void VideoController::loadMovie(Sequence * seq, bool forceCurrentLoad) {
	
	// get the full path of the movie from the sequence
	string path = seq->getMovieFullFilePath();
	LOG_VERBOSE("Next video start to load: " + path + (string)(forceCurrentLoad ? " FORCED" : " NORMAL"));
	
	_forceCurrentLoad = forceCurrentLoad;

	goThreadedVideo * nextMovie			= _appModel->getNextVideoPlayer();
	
	if (_forceCurrentLoad) {
		//nextMovie->close();
		//currentMovie->close();
	}
	
	nextMovie->loadMovie(path);
	
	ofAddListener(nextMovie->loadDone, this, &VideoController::loaded);
	ofAddListener(nextMovie->error, this, &VideoController::error);
	
	setState(kVIDCONTROLLER_NEXTVIDLOADING);

}

void VideoController::toggleVideoPlayers() {
	LOG_VERBOSE("Toggling Video Players");
//	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
//	goThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer();
//	nextMovie->setPosition(0.0f);
	//swap(nextMovie, currentMovie); // why not for?
	_appModel->toggleVideoPlayers(); // swap(_videoPlayers[0], _videoPlayers[1]); should work with (nextMovie, currentMovie) but doesn't!?!?
//	nextMovie->psuedoUpdate();	// now it's the currentMovie...
//	currentMovie->close();		// and this is the last movie!
	

}

void VideoController::loaded(string & path) {
	LOG_VERBOSE("Next video successfully loaded: " + path);
	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	ofRemoveListener(nextMovie->loadDone, this, &VideoController::loaded);
	ofRemoveListener(nextMovie->error, this, &VideoController::error);
	setState(kVIDCONTROLLER_NEXTVIDREADY);
	_cachedLoopAndState = false;
	_forceCurrentLoad = false;
}

void VideoController::error(int & err) {
	LOG_ERROR("Next video error during load: " + ofToString(err));
	setState(kVIDCONTROLLER_NEXTVIDERROR);
	goThreadedVideo * nextMovie			= _appModel->getNextVideoPlayer();
	nextMovie->close();
}