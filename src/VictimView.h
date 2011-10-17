#ifndef _H_VICTIMVIEW
#define _H_VICTIMVIEW

#include "BaseMeterView.h"

class VictimView : public BaseMeterView
{
public:

    VictimView(float width, float height);
    //~VictimView();

    void update();

private:

    float           _scaledInputLevel;

};

#endif // _H_VICTIMVIEW
