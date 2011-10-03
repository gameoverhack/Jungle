#include "VictimView.h"

VictimView::VictimView(float width, float height) : BaseMeterView(width, height) {

	LOG_NOTICE("Setting up VictimView");

    /********************************************************
     *      Set unique x y co ordinates for assets          *
     ********************************************************/

    _turtle_bar_x           = 0.0f;
    _turtle_bar_y           = 0.0f;
    _turtle_x               = 0.0f;
    _turtle_y               = 535.0f;
    _top_x                  = 0.0f;
    _top_y                  = 618.0f;
    _meter_x                = 0.0f;
    _meter_y                = 731.0f;
    _meterSteps             = 16;
    _meterPixelsForStep     = 18.0f;

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    _turtle_bar     = _appModel->getGraphicTex(kGFX_TURTLE_BAR);
    _turtle         = _appModel->getGraphicTex(kGFX_TURTLE);
    _top_on         = _appModel->getGraphicTex(kGFX_TOP_ON);
    _top_off        = _appModel->getGraphicTex(kGFX_TOP_OFF);
    _top_deny       = _appModel->getGraphicTex(kGFX_TOP_DENY);
    _meter_on       = _appModel->getGraphicTex(kGFX_METER_ON);
    _meter_off      = _appModel->getGraphicTex(kGFX_METER_OFF);

    /********************************************************
     *      setup meter FBO and Shader                      *
     ********************************************************/
#if OF_VERSION < 7
    // allocate fbo's
    _meterMaskTex.allocate(147, 285, GL_RGBA); // size of the meter_on/off png
    _meterMaskFBO.setup(147, 285);
    _meterMaskFBO.attach(_meterMaskTex);
#else
    _meterMaskFBO.allocate(147, 285);
#endif
}

void VictimView::update() {

    _scaledInputLevel           = ((float)_appModel->getFFTArea());      // make more complex soon ;-)

    BaseMeterView::update(kINTERACTION_VICTIM);

}
