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

    Fader(float from, float to, float over, FaderType type, bool autoStart = true)
    :
    _from(from),
    _to(to),
    _over(over),
    _type(type),
    _isFading(true){
        if (autoStart) start();
    };

   ~Fader();

    void    update();

    void    start();
    void    stop();
    bool    isFading();
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

};

#endif // _H_FADER
