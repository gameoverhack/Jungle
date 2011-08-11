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

    void drawMeterMask(float level);
    void drawMeterBlend(float x, float y);

    ofxShader       _shader;
    ofTexture       _maskTex;
    ofxFbo          _maskFBO;

    ofTexture *     _meter_on;
    ofTexture *     _meter_off;
    ofTexture *     _meter_level;
    ofTexture *     _icon_on;
    ofTexture *     _icon_off;
    ofTexture *     _icon_bar;

    float           _bar_x;
    float           _icon_x;
    float           _icon_y;
    float           _meter_x;
    float           _meter_y;

    float           _scaledInputLevel;

private:

};

#endif // _H_BASEMETERVIEW
