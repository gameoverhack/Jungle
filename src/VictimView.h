#ifndef _H_VICTIMVIEW
#define _H_VICTIMVIEW

#include "BaseView.h"
#include "ofxShader.h"

class VictimView : public BaseView
{
public:

    VictimView(float width, float height);
    //~VictimView();

    void update();

private:

    void drawMeterMask(float level);
    void drawMeterBlend(float x, float y, ofTexture * meter_on, ofTexture * meter_off);

    ofxShader      _shader;
    ofTexture      _maskTex;
    ofxFbo         _maskFBO;

};

#endif // _H_VICTIMVIEW
