#include "Fader.h"

Fader::~Fader(){
    LOG_NOTICE("Deleting fade");
    stop();
}

void Fader::start(){
    _startTime = ofGetElapsedTimeMillis();
    _fade.time = _startTime;
    _fade.value = _from;
}

bool Fader::isFading(){
    return _isFading;
}

fade Fader::getFade() {
    return _fade;
}

float Fader::getTo() {
    return _to;
}

float Fader::getFrom(){
    return _from;
}

void Fader::stop(){
    _isFading = false;
}

void Fader::update(){

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

    // check if is Fader done
    if (_fade.time >= _over) {
        _fade.value = _to; // conform final value
        _fade.time = _over;
        stop();
    }

}
