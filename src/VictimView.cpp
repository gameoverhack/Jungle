#include "VictimView.h"

VictimView::VictimView(float width, float height) : BaseMeterView(width, height) {

	LOG_NOTICE("Setting up VictimView");

    /********************************************************
     *      Set unique x y co ordinates for assets          *
     ********************************************************/

    _meter_x               = 0.0f;
    _meter_y               = 689.0f;
    _meterSteps             = 16;
    _meterPixelsForStep     = 18.0f;

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    _meter_on      = _appModel->getGraphicTex(kGFX_METER_ON);
    _meter_off     = _appModel->getGraphicTex(kGFX_METER_OFF);

    /********************************************************
     *      setup meter FBO and Shader                      *
     ********************************************************/

    // allocate fbo's
    _maskTex.allocate(147, 285, GL_RGBA); // size of the meter_on/off png
    _maskFBO.setup(147, 285);
    _maskFBO.attach(_maskTex);

}

void VictimView::update() {

    _scaledInputLevel           = ((float)_appModel->getFFTArea());      // make more complex soon ;-)

    BaseMeterView::update(kINTERACTION_VICTIM);

}
