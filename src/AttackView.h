#ifndef _H_ATTACKVIEW
#define _H_ATTACKVIEW

#include "BaseView.h"
#include "ofxShader.h"

class AttackView : public BaseView {

public:

    AttackView(float width, float height);
    //~AttackView();

    void update();

private:

    void drawMeterMask(float level);
    void drawMeterBlend(float x, float y, ofTexture * meter_on, ofTexture * meter_off);

    ofxShader      _shader;
    ofTexture      _maskTex;
    ofxFbo         _maskFBO;

};

#endif // _H_ATTACKVIEW
