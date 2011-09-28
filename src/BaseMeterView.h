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

    void drawMeterMask(float input);
    void drawMeterBlend(float x, float y, float blend = 1.0f);

    ofxShader       _shader;
    ofTexture       _maskTex;
    ofxFbo          _maskFBO;

    ofTexture *     _meter_on;
    ofTexture *     _meter_off;

    float           _meter_x;
    float           _meter_y;

    int             _meterSteps;
    float           _meterPixelsForStep;

    float           _scaledInputLevel;

private:

};

#endif // _H_BASEMETERVIEW
