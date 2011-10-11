#include "SoundController.h"

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

void SoundController::update() {

    Sequence * currentSequence = _appModel->getCurrentSequence();

    int fadeSecs = 2;

    if ((currentSequence->getNumber() == 1 || currentSequence->getNumber() == 8) && currentSequence->getType() != "loop" &&  currentSequence->getType() != "b") {
        if (_appModel->getCurrentSequenceFrame() > currentSequence->getNumFrames() - (12 * fadeSecs) && _currentFade == NULL) {
            fade(1.0, fadeSecs * 1000, FADE_LOG);
        }
        if (_appModel->getCurrentSequenceFrame() > 0 && _appModel->getCurrentSequenceFrame() < 12*fadeSecs-1 && _currentFade == NULL) {
            fade(0.0, fadeSecs * 1000, FADE_LOG);
        }
    }

    if (_currentFade != NULL) {

        setVolume(_currentFade->getFade().value);

        if(!_currentFade->isFading()) {
            delete _currentFade;
            _currentFade = NULL;
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
    _player.setVolume(0.0f);
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
