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

	LOG_VERBOSE("Constructing VideoController");

	registerStates();

	_preRolling = false;
	_forceCurrentLoad = false;
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

	_appModel->setCurrentSequenceFrame(currentFrame);
	//_appModel->setCurrentFrameTotal(totalFrames); // no need for now?
	_appModel->setCurrentIsFrameNew(isFrameNew);

	if (currentFrame > totalFrames - 12 && !_preRolling) {

        string nextResult = currentSequence->getNextResult()[currentFrame]; // technically all members in getNextResult should point to the same next sequence name...

		// set loopstate
		if (nextResult == currentSequence->getName()) { //(currentSequence->getType() == "loop") {
			currentMovie->setLoopState(OF_LOOP_NORMAL);
		} else {
			currentMovie->setLoopState(OF_LOOP_NONE);
		}

		// 'cache' (ie., half) load loop sequence if we're on an 'a' type movie
        if (nextResult == "__RETURN_SEQUENCE__") nextResult = _lastSequenceWhenForced; // return to whence thy came ;-) this is for b movies
		if (nextResult != "" && nextResult != "__FINAL_SEQUENCE__" && nextResult != currentSequence->getName()) {
		    LOG_NOTICE("End of last mov start: " + nextResult);
			loadMovie(currentScene->getSequence(nextResult));
			_preRolling = true;
		} else if (nextResult == "__FINAL_SEQUENCE__") {
			// end of SCENE? which seq to cache?
			_preRolling = false;
		}

	}

	if (currentMovie->getIsMovieDone()) {
		toggleVideoPlayers(_lastFrameWhenForced);
		_lastFrameWhenForced = 0;
		_preRolling = false;
		setState(kVIDCONTROLLER_CURRENTVIDONE);
	}

}

void VideoController::forceUpdate() {
	// used to force loading of a movie if we're not actually updating/drawing it
	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	nextMovie->psuedoUpdate();
	nextMovie->psuedoDraw();
}

void VideoController::loadMovie(Sequence * seq, bool forceCurrentLoad, int lastFrameWhenForced) {

	// get the full path of the movie from the sequence
	string path = seq->getMovieFullFilePath();
	LOG_VERBOSE("Next video start to load: " + path + (string)(forceCurrentLoad ? " FORCED: " + ofToString(lastFrameWhenForced) : " NORMAL"));

	_forceCurrentLoad       = forceCurrentLoad;
    if (lastFrameWhenForced != 0) {
        _lastFrameWhenForced    = lastFrameWhenForced;
        _lastSequenceWhenForced = _appModel->getCurrentSequence()->getName();
    }

	goThreadedVideo * nextMovie			= _appModel->getNextVideoPlayer();

	nextMovie->loadMovie(path);

	ofAddListener(nextMovie->loadDone, this, &VideoController::loaded);
	ofAddListener(nextMovie->error, this, &VideoController::error);

	setState(kVIDCONTROLLER_NEXTVIDLOADING);

}

void VideoController::toggleVideoPlayers(int lastFrameWhenForced) {
	LOG_NOTICE("Toggling Video Players " + ofToString(lastFrameWhenForced));
//	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
//	goThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer();
//	nextMovie->setPosition(0.0f);
	//swap(nextMovie, currentMovie); // why not for?
	_appModel->toggleVideoPlayers(lastFrameWhenForced); // swap(_videoPlayers[0], _videoPlayers[1]); should work with (nextMovie, currentMovie) but doesn't!?!?
//	nextMovie->psuedoUpdate();	// now it's the currentMovie...
//	currentMovie->close();		// and this is the last movie!


}

void VideoController::loaded(string & path) {
	LOG_NOTICE("Next video successfully loaded: " + path);
	goThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	ofRemoveListener(nextMovie->loadDone, this, &VideoController::loaded);
	ofRemoveListener(nextMovie->error, this, &VideoController::error);
	_preRolling = false;
	_forceCurrentLoad = false;
	setState(kVIDCONTROLLER_NEXTVIDREADY);
}

void VideoController::error(int & err) {
	LOG_ERROR("Next video error during load: " + ofToString(err));
	setState(kVIDCONTROLLER_NEXTVIDERROR);
	goThreadedVideo * nextMovie			= _appModel->getNextVideoPlayer();
	nextMovie->close();
}

bool VideoController::isPreRolling() {
	return _preRolling;
}
