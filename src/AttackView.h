#ifndef _H_ATTACKVIEW
#define _H_ATTACKVIEW

#include "BaseMeterView.h"

class AttackView : public BaseMeterView {

public:

    AttackView(float width, float height);
    //~AttackView();

    void update();

private:

    float           _scaledInputLevel;

};

#endif // _H_ATTACKVIEW
