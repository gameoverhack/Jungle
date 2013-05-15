#ifndef _H_FADER
#define _H_FADER

#include "Logger.h"
#include "ofEvents.h"

enum FaderType {
    FADE_LINEAR,
    FADE_LOG,
    FADE_EXP
};

typedef struct {
    float time;
    float value;
} fade;

class Fader{

public:

    Fader(){};

    Fader(float from, float to, float over, FaderType type, bool autoStart = false)
    :
    _from(from),
    _to(to),
    _over(over),
    _type(type),
    _autoStart(autoStart){
        reset();
    };

   ~Fader();

    void    update();
    void    reset();

    void    start();
    void    stop();
    bool    isFading();
    bool    isFinished();
    fade    getFade();
    float   getTo();
    float   getFrom();

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
    bool    _autoStart;

};

#endif // _H_FADER
