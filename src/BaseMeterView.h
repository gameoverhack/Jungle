#ifndef _H_BASEMETERVIEW
#define _H_BASEMETERVIEW

#include "BaseView.h"
#if OF_VERSION < 7
#include "ofxShader.h"
#endif

class BaseMeterView : public BaseView
{
public:

    BaseMeterView(float width, float height);
    virtual ~BaseMeterView();

    void    update() = 0;

protected:

    float   _scaledInputLevel;
    bool    _isFlashing;

#if OF_VERSION < 7
    void drawMeterMask(float input, int meterSteps, float meterPixelsForStep, ofxFbo * maskFBO);
    void drawMeterMask(int input, int meterSteps, float meterPixelsForStep, ofxFbo * maskFBO);
#else
    void drawMeterMask(float input, int meterSteps, float meterPixelsForStep, ofFbo * maskFBO);
    void drawMeterMask(int input, int meterSteps, float meterPixelsForStep, ofFbo * maskFBO);
#endif
    void drawMeterShader(float x, float y, ofTexture *_maskTex, ofTexture *meterOnTex, ofTexture *meterOffTex);

#if OF_VERSION < 7
    ofxShader       _shader;
    ofxFbo          _meterMaskFBO;
    ofxFbo          _stationMaskFBO;
    ofTexture       _meterMaskTex;
    ofTexture       _stationMaskTex;
#else
    ofShader        _shader;
    ofFbo           _meterMaskFBO;
    ofFbo           _stationMaskFBO;
#endif

    ofTexture *     _meter_on;
    ofTexture *     _meter_off;
    ofTexture *     _top_on;
    ofTexture *     _top_off;
    ofTexture *     _top_deny;
    ofTexture *     _button_on;
    ofTexture *     _button_off;
    ofTexture *     _button_deny;
    ofTexture *     _stations_on;
    ofTexture *     _stations_off;
    ofTexture *     _turtle_bar;
    ofTexture *     _turtle;
    ofTexture *     _bird;
    ofTexture *     _button_flash;
    ofTexture *     _face_flash;

    float           _top_x;
    float           _top_y;
    float           _button_x;
    float           _button_y;
    float           _stations_x;
    float           _stations_y;
    float           _turtle_bar_x;
    float           _turtle_bar_y;
    float           _turtle_x;
    float           _turtle_y;
    float           _bird_x;
    float           _bird_y;
    float           _meter_x;
    float           _meter_y;

    int             _meterSteps;
    float           _meterPixelsForStep;
    int             _stationSteps;
    float           _stationPixelsForStep;

};

#endif // _H_BASEMETERVIEW
