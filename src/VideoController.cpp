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
	_lastFrameWhenForced = 0;
	/* setup listeners for error and loadDone
	ofxThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	ofxThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer();

	ofAddListener(nextMovie->loadDone, this, &VideoController::loaded);
	ofAddListener(nextMovie->error, this, &VideoController::error);
	ofAddListener(currentMovie->loadDone, this, &VideoController::loaded);
	ofAddListener(currentMovie->error, this, &VideoController::error);*/

}

VideoController::~VideoController() {

	LOG_VERBOSE("Destroying VideoController and removing listeners");

	/* remove listeners for error and loadDone
	ofxThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	ofxThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer();

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

	// update the current currentMovie/videoplayer

	Scene			* currentScene		= _appModel->getCurrentScene();
	Sequence		* currentSequence	= _appModel->getCurrentSequence();
	ofxThreadedVideo * currentMovie		= _appModel->getCurrentVideoPlayer();
    ofxThreadedVideo * nextMovie        = _appModel->getNextVideoPlayer();

    if (_forceCurrentLoad || _preRolling) nextMovie->update();
	currentMovie->update();

    if (currentMovie->getVolume() < 256) currentMovie->setVolume(256);

	// set global vars for current frame and isFrameNew on the Model -> hopefully eliminates misreadings???
	int	 totalFrames	= _appModel->getCurrentSequenceNumFrames();
	int	 currentFrame	= currentMovie->getCurrentFrame();
	bool isFrameNew		= currentMovie->isFrameNew();

	_appModel->setCurrentSequenceFrame(currentFrame);
	//_appModel->setCurrentFrameTotal(totalFrames); // no need for now?
	_appModel->setCurrentIsFrameNew(isFrameNew);

    string nextResult = "";

	if (currentFrame > totalFrames - 12 && !_preRolling && !checkState(kVIDCONTROLLER_NEXTVIDLOADING)) {

        nextResult = currentSequence->getNextResult()[currentFrame]; // technically all members in getNextResult should point to the same next sequence name...

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
    if(checkState(kVIDCONTROLLER_NEXTVIDREADY)){
        if (currentMovie->getIsMovieDone()) {
            if (_preRolling) {
                cout << "maybe" << endl;
                nextMovie->update();
                if(nextMovie->isLoaded() && nextMovie->getCurrentFrame() > _lastFrameWhenForced){
                    toggleVideoPlayers();
                    _lastFrameWhenForced = 0;
                    _preRolling = false;
                    setState(kVIDCONTROLLER_CURRENTVIDONE);
                }
            }
        }
    }

    if(checkState(kVIDCONTROLLER_READY)){
        if (currentMovie->getIsMovieDone()) {
            if (nextResult == "__FINAL_SEQUENCE__") {
                cout << "maybewhtfs" << endl;
                setState(kVIDCONTROLLER_CURRENTVIDONE);
            }
        }
    }

    if(checkState(kVIDCONTROLLER_NEXTVIDLOADING)){
        cout << "maybenot" << endl;
        if(currentMovie == NULL || nextMovie == NULL) {
            cout << "something is null; bailing" << endl;
            return;
        }
        if(nextMovie->isLoaded() && nextMovie->getCurrentFrame() > 0){
            _forceCurrentLoad = false;
            setState(kVIDCONTROLLER_NEXTVIDREADY);
        }
    }



}

void VideoController::loadMovie(Sequence * seq, bool forceCurrentLoad, int lastFrameWhenForced) {

    _appModel->restartTimer("anyActionTimer");

	// get the full path of the movie from the sequence
	string path = seq->getMovieFullFilePath();
	LOG_VERBOSE("Next video start to load: " + path + (string)(forceCurrentLoad ? " FORCED: " + ofToString(lastFrameWhenForced) : " NORMAL"));

	ofxThreadedVideo * nextMovie			= _appModel->getNextVideoPlayer();
	nextMovie->loadMovie(path);
    if(_lastFrameWhenForced != 0) nextMovie->setFrame(_lastFrameWhenForced);

	_forceCurrentLoad       = forceCurrentLoad;
    if (lastFrameWhenForced != 0) {
        _lastFrameWhenForced    = lastFrameWhenForced;
        _lastSequenceWhenForced = _appModel->getCurrentSequence()->getName();
    }

	ofAddListener(nextMovie->threadedVideoEvent, this, &VideoController::videoEvent);

	setState(kVIDCONTROLLER_NEXTVIDLOADING);

}

void VideoController::toggleVideoPlayers(int lastFrameWhenForced, bool noPause) {
	LOG_NOTICE("Toggling Video Players " + ofToString(lastFrameWhenForced));
    _appModel->restartTimer("anyActionTimer");
	_appModel->toggleVideoPlayers(lastFrameWhenForced, noPause);
	//update();
}

void VideoController::reset() {
    _preRolling = false;
	_forceCurrentLoad = false;
	_lastFrameWhenForced = 0;
	ofxThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
	ofxThreadedVideo * currentMovie = _appModel->getCurrentVideoPlayer();
    ofRemoveListener(nextMovie->threadedVideoEvent, this, &VideoController::videoEvent);
    ofRemoveListener(currentMovie->threadedVideoEvent, this, &VideoController::videoEvent);
	nextMovie->close();
	currentMovie->close();
	//update();
}

void VideoController::videoEvent(ofxThreadedVideoEvent & event) {
    string path = event.path;
    switch(event.eventType){
        case VIDEO_EVENT_LOAD_OK:
        {
            LOG_NOTICE("Next video successfully loaded: " + path);
            ofxThreadedVideo * nextMovie = _appModel->getNextVideoPlayer();
            ofRemoveListener(nextMovie->threadedVideoEvent, this, &VideoController::videoEvent);
            break;
        }
        default:
        {
            LOG_ERROR("Next video error during load: " + event.eventTypeAsString);
            setState(kVIDCONTROLLER_NEXTVIDERROR);
            break;
        }
    }

}


bool VideoController::isPreRolling() {
	return _preRolling;
}
