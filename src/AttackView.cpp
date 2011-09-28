#include "AttackView.h"

AttackView::AttackView(float width, float height) : BaseMeterView(width, height) {

    LOG_NOTICE("Setting up AttackView");

    /********************************************************
     *      Set unique x y co ordinates for assets          *
     ********************************************************/
    _meter_x                = 0.0f;
    _meter_y                = 731.0f;
    _meterSteps             = 5;
    _meterPixelsForStep     = 45.0f;

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    _meter_on      = _appModel->getGraphicTex(kGFX_ARROWS_ON);
    _meter_off     = _appModel->getGraphicTex(kGFX_ARROWS_OFF);
    //_meter_level   = _appModel->getGraphicTex(kGFX_METER_LEVEL);

    /********************************************************
     *      setup meter FBO and Shader                      *
     ********************************************************/

    // allocate fbo's
    _maskTex.allocate(147, 225, GL_RGBA); // size of the meter_on/off png
    _maskFBO.setup(147, 225);
    _maskFBO.attach(_maskTex);

}

void AttackView::update() {

    _scaledInputLevel           = (float)_appModel->getARDArea(); //(float)_appModel->getPinInput()[0]/600.0f;      // make more complex soon ;-)

    BaseMeterView::update(kINTERACTION_ATTACKER);

}
