#ifndef H_SOUNDCONTROLLER
#define H_SOUNDCONTROLLER

#include "AppModel.h"
#include "Fader.h"

class JungleFade {

public:

    JungleFade(){};

    JungleFade(string sequenceName, float startValue, float endValue, int startFrame, int endFrame, int duration, FaderType type):
    _sequenceName(sequenceName),
    _startValue(startValue),
    _endValue(endValue),
    _startFrame(startFrame),
    _endFrame(endFrame),
    _duration(duration),
    _type(type),
    _done(false)
    {
        _fader = Fader(startValue, endValue, duration, type, false);
    };

    string _sequenceName;
    float _startValue;
    float _endValue;
    int _startFrame;
    int _endFrame;
    int _duration;
    FaderType _type;
    Fader _fader;
    bool _done;

};

class SoundController {

public:

    SoundController();
    ~SoundController();

    void    setup();
    void    reset();
    void    update();
    void    loadSound();

    //void    fade(float toVolume, int timeMillis, FaderType fadeType);

    void    setVolume(float volume);

private:

    Fader                   _currentFade;

    ofSoundPlayer           _player;

    map<string, vector<JungleFade> >    _allFades;
    int                                 _fadeIndex;

};

#endif // H_SOUNDCONTROLLER
