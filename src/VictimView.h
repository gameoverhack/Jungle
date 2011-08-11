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

    ofxShader _shader;

};

#endif // _H_VICTIMVIEW
