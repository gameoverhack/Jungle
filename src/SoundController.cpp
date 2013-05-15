#include "SoundController.h"

static int sFades[17] = {17,32,3167,250,12,50,2083,344,12,201,4917,552,12,132,5583,473,12};
static int fFades[13] = {13,12,3250,234,12,68,1250,316,12,69,4667,447,12};
static int tFades[17] = {17,6,3000,215,12,106,2560,296,12,12,5500,445,12,14,5417,445,12};

SoundController::SoundController() {
    LOG_NOTICE("Constructing SoundController");
//    _currentFade = NULL;
}

SoundController::~SoundController() {
    LOG_NOTICE("Destroying SoundController");

//    if (_currentFade != NULL) {
//        delete _currentFade;
//        _currentFade = NULL;
//    }

    _allFades.clear();

    _player.stop();
    _player.unloadSound();

}

void SoundController::setup() {

    LOG_NOTICE("Setting up sound fades for all scenes");

    _allFades.clear();

    string currentSceneName;

    currentSceneName = "s";

    vector<JungleFade> sfades;
    vector<JungleFade> ffades;
    vector<JungleFade> tfades;

    for (int i = 1; i < sFades[0]; i+=4) {

        string seqName;
        if (i == 1) seqName  = "seq01a";
        if (i == 5) seqName  = "seq08a";
        if (i == 9) seqName  = "seq08b0";
        if (i == 13) seqName = "seq08b1";

        LOG_VERBOSE("Adding fade to que: " + seqName + "::"+ ofToString(sFades[i]) + "::" + ofToString(sFades[i+1]) + "::" + ofToString(sFades[i+2]) + "::" + ofToString(sFades[i+3]));
//string sequenceName, float startValue, float endValue, int startFrame, int endFrame, int duration, FaderType type
        JungleFade j1 = JungleFade(seqName, 1.0f, 0.0f, sFades[i], sFades[i] + ceil(sFades[i+1]/1000*12.0), sFades[i+1], FADE_LOG);
        JungleFade j2 = JungleFade(seqName, 0.0f, 1.0f, sFades[i+2], sFades[i+2] + ceil(sFades[i+3]/1000*12.0), sFades[i+3], FADE_LINEAR);

        sfades.push_back(j1);
        sfades.push_back(j2);
    }

    for (int i = 1; i < fFades[0]; i+=4) {

        string seqName;
        if (i == 1) seqName  = "seq01a";
        if (i == 5) seqName  = "seq08a";
        if (i == 9) seqName  = "seq08b0";

        LOG_VERBOSE("Adding fade to que: " + seqName + "::"+ ofToString(fFades[i]) + "::" + ofToString(fFades[i+1]) + "::" + ofToString(fFades[i+2]) + "::" + ofToString(fFades[i+3]));

        JungleFade j1 = JungleFade(seqName, 1.0f, 0.0f, fFades[i], fFades[i] + ceil(fFades[i+1]/1000*12.0), fFades[i+1], FADE_LOG);
        JungleFade j2 = JungleFade(seqName, 0.0f, 1.0f, fFades[i+2], fFades[i+2] + ceil(fFades[i+3]/1000*12.0), fFades[i+3], FADE_LINEAR);

        ffades.push_back(j1);
        ffades.push_back(j2);

    }

    for (int i = 1; i < tFades[0]; i+=4) {

        string seqName;
        if (i == 1) seqName  = "seq01a";
        if (i == 5) seqName  = "seq08a";
        if (i == 9) seqName  = "seq08b0";
        if (i == 13) seqName = "seq08b1";

        LOG_VERBOSE("Adding fade to que: " + seqName + "::"+ ofToString(tFades[i]) + "::" + ofToString(tFades[i+1]) + "::" + ofToString(tFades[i+2]) + "::" + ofToString(tFades[i+3]));

        JungleFade j1 = JungleFade(seqName, 1.0f, 0.0f, tFades[i], tFades[i] + ceil(tFades[i+1]/1000*12.0), tFades[i+1], FADE_LOG);
        JungleFade j2 = JungleFade(seqName, 0.0f, 1.0f, tFades[i+2], tFades[i+2] + ceil(tFades[i+3]/1000*12.0), tFades[i+3], FADE_LINEAR);

        tfades.push_back(j1);
        tfades.push_back(j2);

    }

    _allFades["s"] = sfades;
    _allFades["f"] = ffades;
    _allFades["t"] = tfades;

}

void SoundController::reset(){
    setVolume(1.0);
    Scene * currentScene            = _appModel->getCurrentScene();
    string currentSceneName         = currentScene->getName();
    vector<JungleFade> & fades      = _allFades[currentSceneName];
    for(int i = 0; i < fades.size(); i++){
        Fader & currentFade = fades[i]._fader;
        currentFade.reset();
    }
}

void SoundController::update() {

    Scene * currentScene            = _appModel->getCurrentScene();
    string currentSceneName         = currentScene->getName();
    Sequence * currentSequence      = _appModel->getCurrentSequence();
    string currentSequenceName      = currentSequence->getName();
    int currentSequenceFrame        = _appModel->getCurrentSequenceFrame();

    vector<JungleFade> & fades = _allFades[currentSceneName];

    for(int i = 0; i < fades.size(); i++){
        Fader & currentFade = fades[i]._fader;
        //cout << fades[i]._startFrame << " " << currentSequenceFrame << " " << fades[i]._endFrame << endl;
        if (currentSequenceName == fades[i]._sequenceName &&
            currentSequenceFrame >= fades[i]._startFrame){
                //cout << fades[i]._startFrame << " " << currentSequenceFrame << " " << fades[i]._endFrame << endl;
                if(!currentFade.isFading() && !currentFade.isFinished()){
                    LOG_VERBOSE("Rewind");
                    _player.setPosition(0.0);
                    currentFade.start();
                }
                if(currentFade.isFading()){
                    currentFade.update();
                    LOG_VERBOSE("Fading: " + fades[i]._sequenceName + " " + ofToString(currentFade.getFade().value));
                    setVolume(currentFade.getFade().value);
                }
        }
    }

//    if (_currentFade != NULL) {
//        //execute current fade
//        _currentFade->update();
//        LOG_VERBOSE("Executing fade: " + ofToString(_currentFade->getFade().value));
//        setVolume(_currentFade->getFade().value);
//
//        if(!_currentFade->isFading()) {
//            _fades.erase(_fades.begin()+_fadeIndex);
//            delete _currentFade;
//            _currentFade = NULL;
//        }
//    }
//
//    if (_currentFade == NULL && _fades.size() > 0) {
//        //cout << "sound4" << endl;
//        //check if we have a fade to start
//        for (int i = 0; i < _fades.size(); i++) {
//            //cout << "sound5" << endl;
//            jungle_fade * nextFade = _fades[i];
//            //LOG_VERBOSE("Check new fade " + currentSequenceName + " ? " + nextFade->sequenceName + " " + ofToString(currentSequenceFrame) + " ? " + ofToString(nextFade->startFrame));
//            if (currentSequenceName == nextFade->sequenceName && currentSequenceFrame >= nextFade->startFrame && !nextFade->done) {
//                LOG_VERBOSE("Start Fade " + ofToString(i) + " que: " + _fades[i]->sequenceName + "::"+ ofToString(_fades[i]->startFrame) + "::" + ofToString(_fades[i]->endFrame));
//                _currentFade = new Fader(nextFade->startValue, nextFade->endValue, nextFade->duration, nextFade->type, false);
//                if (_currentFade->getTo() == 1.0) {
//                    LOG_VERBOSE("Rewind");
//                    _player.setPosition(0.0);
//                }
//                _currentFade->start();
//                _fadeIndex = i;
//                nextFade->done = true;
//                break;
//            }
//        }
//    }
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

//void SoundController::fade(float toVolume, int timeMillis, FaderType fadeType) {
//
////    if(_currentFade != NULL) {
////        LOG_WARNING("Fade is already in progress...killing it");
////        delete _currentFade;
////        _currentFade = NULL;
////    }
////
////    LOG_VERBOSE("New Fade from " + ofToString(toVolume) + " over " + ofToString(timeMillis));
////    _currentFade = new Fader(_player.getVolume(), toVolume, timeMillis, fadeType);
//
//}

void SoundController::setVolume(float volume) {
    _player.setVolume(volume);
}
