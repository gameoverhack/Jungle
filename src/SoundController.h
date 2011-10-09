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

    Fader(float from, float to, float over, FaderType type)
    :
    _from(from),
    _to(to),
    _over(over),
    _type(type),
    _startTime(ofGetElapsedTimeMillis()),
    _isFading(true) {
        _fade.time = _startTime;
        _fade.value = _from;
        startThread(false, false);
    };

    ~Fader() {
        if (isThreadRunning()) stopThread();
    };

    bool    isFading() {return _isFading;};
    fade   getFade() {return _fade;};

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
                float base = 10;
                _pct = -pow(base, (_now * 1.04 - _over)/_over) + 1.09;
                break;
            }
        }

        _fade.value = (_pct * (_from - _to) + _to);
        _fade.time  = _now;
    }

};

class SoundController {

public:

    SoundController();
    ~SoundController();

    void    update();
    void    loadSound(Sequence * seq);

    void    fade(float toVolume, int timeMillis, FaderType fadeType);

private:

    void    setVolume(float volume);

    Fader*          _currentFade;

    ofSoundPlayer*  _player;

};

#endif // H_SOUNDCONTROLLER
