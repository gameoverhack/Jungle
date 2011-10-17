#include "SoundController.h"

static int sFades[17] = {17,32,3167,250,1,50,2083,344,1,201,4917,552,1,132,5583,473,1};
static int fFades[13] = {13,1,3250,234,1,68,1250,316,1,69,4667,447,1};
static int tFades[17] = {17,6,3000,215,1,106,2560,296,1,12,5500,445,1,14,5417,445,1};

SoundController::SoundController() {
    LOG_NOTICE("Constructing SoundController");
    _currentFade = NULL;
}

SoundController::~SoundController() {
    LOG_NOTICE("Destroying SoundController");

    if (_currentFade != NULL) {
        delete _currentFade;
        _currentFade = NULL;
    }

    _player.stop();
    _player.unloadSound();

}

void SoundController::setup() {

    Scene * currentScene    = _appModel->getCurrentScene();
    string currentSceneName = currentScene->getName();

    LOG_NOTICE("Setting up sound fades for scene: " + currentSceneName);

    if(_currentFade != NULL) {
        delete _currentFade;
        _currentFade = NULL;
    }

//    for (int i = 0; i < _fades.size(); i++) {
//        LOG_VERBOSE("Deleting old fade");
//        jungle_fade * j = _fades[i];
//        if (j->fader != NULL) delete j->fader;
//        delete j;
//    }

    _fades.clear();

    if (currentSceneName == "s") {

        for (int i = 1; i < sFades[0]; i+=4) {

            string seqName;
            if (i == 1) seqName  = "seq01a";
            if (i == 5) seqName  = "seq08a";
            if (i == 9) seqName  = "seq08b0";
            if (i == 13) seqName = "seq08b1";

            LOG_VERBOSE("Adding fade to que: " + seqName + "::"+ ofToString(sFades[i]) + "::" + ofToString(sFades[i+1]) + "::" + ofToString(sFades[i+2]) + "::" + ofToString(sFades[i+3]));

            jungle_fade * j1 = new jungle_fade(); j1->sequenceName = seqName; j1->startFrame = sFades[i]; j1->endFrame = j1->startFrame + sFades[i+1]; j1->done = false;
            j1->fader = new Fader(1.0, 0.0, sFades[i+1], FADE_LOG, false);

            jungle_fade * j2 = new jungle_fade(); j2->sequenceName = seqName; j2->startFrame = sFades[i+2]; j2->endFrame = j2->startFrame + sFades[i+3]; j2->done = false;
            j2->fader = new Fader(0.0, 1.0, sFades[i+3], FADE_LINEAR, false);

            _fades.push_back(j1);
            _fades.push_back(j2);
        }

    }

    if (currentSceneName == "f") {


        for (int i = 1; i < fFades[0]; i+=4) {

            string seqName;
            if (i == 1) seqName  = "seq01a";
            if (i == 5) seqName  = "seq08a";
            if (i == 9) seqName  = "seq08b0";

            LOG_VERBOSE("Adding fade to que: " + seqName + "::"+ ofToString(fFades[i]) + "::" + ofToString(fFades[i+1]) + "::" + ofToString(fFades[i+2]) + "::" + ofToString(fFades[i+3]));

            jungle_fade * j1 = new jungle_fade(); j1->sequenceName = seqName; j1->startFrame = fFades[i]; j1->endFrame = j1->startFrame + fFades[i+1]; j1->done = false;
            j1->fader = new Fader(1.0, 0.0, fFades[i+1], FADE_LOG, false);

            jungle_fade * j2 = new jungle_fade(); j2->sequenceName = seqName; j2->startFrame = fFades[i+2]; j2->endFrame = j2->startFrame + fFades[i+3]; j2->done = false;
            j2->fader = new Fader(0.0, 1.0, fFades[i+3], FADE_LINEAR, false);

            _fades.push_back(j1);
            _fades.push_back(j2);
        }
    }

    if (currentSceneName == "t") {

        for (int i = 1; i < tFades[0]; i+=4) {

            string seqName;
            if (i == 1) seqName  = "seq01a";
            if (i == 5) seqName  = "seq08a";
            if (i == 9) seqName  = "seq08b0";
            if (i == 13) seqName = "seq08b1";

            LOG_VERBOSE("Adding fade to que: " + seqName + "::"+ ofToString(tFades[i]) + "::" + ofToString(tFades[i+1]) + "::" + ofToString(tFades[i+2]) + "::" + ofToString(tFades[i+3]));

            jungle_fade * j1 = new jungle_fade(); j1->sequenceName = seqName; j1->startFrame = tFades[i]; j1->endFrame = j1->startFrame + tFades[i+1]; j1->done = false;
            j1->fader = new Fader(1.0, 0.0, tFades[i+1], FADE_LOG, false);

            jungle_fade * j2 = new jungle_fade(); j2->sequenceName = seqName; j2->startFrame = tFades[i+2]; j2->endFrame = j2->startFrame + tFades[i+3]; j2->done = false;
            j2->fader = new Fader(0.0, 1.0, tFades[i+3], FADE_LINEAR, false);

            _fades.push_back(j1);
            _fades.push_back(j2);
        }
    }

    for (int i = 0; i < _fades.size(); i++) {
        LOG_VERBOSE("Fade " + ofToString(i) + " que: " + _fades[i]->sequenceName + "::"+ ofToString(_fades[i]->startFrame) + "::" + ofToString(_fades[i]->endFrame));
    }
}

void SoundController::update() {

    Sequence * currentSequence      = _appModel->getCurrentSequence();
    string currentSequenceName      = currentSequence->getName();
    int currentSequenceFrame        = _appModel->getCurrentSequenceFrame();

    if (_currentFade != NULL) {

        //execute current fade
        LOG_VERBOSE("Executing fade: " + ofToString(_currentFade->getFade().value));
        setVolume(_currentFade->getFade().value);

        if(!_currentFade->isFading()) {
            delete _currentFade;
            _currentFade = NULL;
            _fades.erase(_fades.begin()+_fadeIndex);
        }
    } else if (_currentFade == NULL && _fades.size() > 0) {

        //check if we have a fade to start
        for (int i = 0; i < _fades.size(); i++) {
            jungle_fade * nextFade = _fades[i];
            //LOG_VERBOSE("Check new fade " + currentSequenceName + " ? " + nextFade->sequenceName + " " + ofToString(currentSequenceFrame) + " ? " + ofToString(nextFade->startFrame));
            if (currentSequenceName == nextFade->sequenceName && currentSequenceFrame >= nextFade->startFrame && !nextFade->done) {
                LOG_VERBOSE("Start Fade " + ofToString(i) + " que: " + _fades[i]->sequenceName + "::"+ ofToString(_fades[i]->startFrame) + "::" + ofToString(_fades[i]->endFrame));
                _currentFade = nextFade->fader;
                if (_currentFade->getTo() == 1.0) {
                    LOG_VERBOSE("Rewind");
                    _player.setPosition(0.0);
                }
                _currentFade->start();
                _fadeIndex = i;
                nextFade->done = true;
                break;
            }
        }




    }
}

void SoundController::loadSound() {
    LOG_NOTICE("Attempting to load scene background sound...");
    if (_player.getIsPlaying()) _player.stop();
    string path = boost::any_cast<string>(_appModel->getProperty("audioDataPath")) + "/" + "backgroundAudio.wav"; // + scene->getName() + ".wav";
    LOG_NOTICE(path);
    _player.loadSound(path);
    _player.setLoop(true);
    _player.setVolume(1.0f);
    _player.play();
}

void SoundController::fade(float toVolume, int timeMillis, FaderType fadeType) {

    if(_currentFade != NULL) {
        LOG_WARNING("Fade is already in progress...killing it");
        delete _currentFade;
        _currentFade = NULL;
    }

    LOG_VERBOSE("New Fade from " + ofToString(toVolume) + " over " + ofToString(timeMillis));
    _currentFade = new Fader(_player.getVolume(), toVolume, timeMillis, fadeType);

}

void SoundController::setVolume(float volume) {
    _player.setVolume(volume);
}
