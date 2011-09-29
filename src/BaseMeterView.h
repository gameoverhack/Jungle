#ifndef _H_BASEMETERVIEW
#define _H_BASEMETERVIEW

#include "BaseView.h"
#include "ofxShader.h"

class BaseMeterView : public BaseView
{
public:

    BaseMeterView(float width, float height);
    virtual ~BaseMeterView();

    void    update(interaction_t interactionType);

protected:

    void drawMeterMask(float input, int meterSteps, float meterPixelsForStep, ofxFbo * maskFBO);
    void drawMeterMask(int input, int meterSteps, float meterPixelsForStep, ofxFbo * maskFBO);
    void drawMeterShader(float x, float y, ofTexture *_maskTex, ofTexture *meterOnTex, ofTexture *meterOffTex);

    ofxShader       _shader;
    ofTexture       _meterMaskTex;
    ofxFbo          _meterMaskFBO;
    ofTexture       _stationMaskTex;
    ofxFbo          _stationMaskFBO;

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

    float           _scaledInputLevel;

private:

};

#endif // _H_BASEMETERVIEW
