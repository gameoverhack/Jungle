#ifndef H_SOUNDCONTROLLER
#define H_SOUNDCONTROLLER

#include "AppModel.h"

enum FaderType {
    FADE_LINEAR,
    FADE_LOG,
    FADE_EXP
};

typedef struct {
    float time;
    float value;
} fade;

class Fader : public ofThread {

public:

    Fader(float from, float to, float over, FaderType type, bool autoStart = true)
    :
    _from(from),
    _to(to),
    _over(over),
    _type(type),
    _isFading(true) {
        if (autoStart) start();
    };

    ~Fader() {
        LOG_NOTICE("Deleting fade");
        if (isThreadRunning()) stopThread();
    };

    void   start() {
        _startTime = ofGetElapsedTimeMillis();
        _fade.time = _startTime;
        _fade.value = _from;
        startThread(false, false);
    };
    bool   isFading() {return _isFading;};
    fade   getFade() {return _fade;};
    float  getTo() {return _to;};
    float  getFrom() {return _from;};

private:

    float   _from;
    float   _to;
    fade    _fade;
    float   _pct;

    float   _over;
    float   _startTime;
    int     _now;

    int     _type;
    bool    _isFading;

    void threadedFunction() {
        while(isThreadRunning() != 0) {
            if(lock()) {

                calculate();
                // check if is Fader done
                if (_fade.time >= _over) {
                    _fade.value = _to; // conform final value
                    _fade.time = _over;
                    _isFading = false;
                    stopThread();
                }

            }
        }
    }

    inline void calculate() {

        _now = ofGetElapsedTimeMillis() - _startTime;

        switch (_type) {
            case FADE_LINEAR:
            {
                _pct = 1.0 - (_now / _over);
                break;
            }
            case FADE_LOG:
            {
                float base = 10; //2.71828182845904523536028747135266249775724709369995;
                _pct = log10(_over/(_over + (base-1) * _now)) + 1;
                break;
            }
            case FADE_EXP:
            {
                float base = 10.0f;
                _pct = -pow((double)base, (double)((_now * 1.04 - _over)/_over)) + 1.09;
                break;
            }
        }

        _fade.value = (_pct * (_from - _to) + _to);
        _fade.time  = _now;
    }

};

typedef struct {
    Fader* fader;
    string sequenceName;
    int startFrame;
    int endFrame;
    bool done;
} jungle_fade;

class SoundController {

public:

    SoundController();
    ~SoundController();

    void    setup();
    void    update();
    void    loadSound();

    void    fade(float toVolume, int timeMillis, FaderType fadeType);

    void    setVolume(float volume);

private:

    Fader*              _currentFade;

    ofSoundPlayer       _player;

    vector<jungle_fade*>  _fades;
    int                   _fadeIndex;

};

#endif // H_SOUNDCONTROLLER
