#include "AttackView.h"

AttackView::AttackView(float width, float height) : BaseMeterView(width, height) {

    LOG_NOTICE("Setting up AttackView");

    /********************************************************
     *      Set unique x y co ordinates for assets          *
     ********************************************************/

    _bird_x                 = 0.0f;
    _bird_y                 = 0.0f;
    _stations_x             = 0.0f;
    _stations_y             = 134.0f;
    _button_x               = 0.0f;
    _button_y               = 921.0f;
    _meter_x                = 0.0f;
    _meter_y                = 731.0f;
    _meterSteps             = 5;
    _meterPixelsForStep     = 45.0f;
    _stationSteps           = 7;
    _stationPixelsForStep   = 74.0f;

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    _bird           = _appModel->getGraphicTex(kGFX_BIRD);
    _stations_on    = _appModel->getGraphicTex(kGFX_STATIONS_ON);
    _stations_off   = _appModel->getGraphicTex(kGFX_STATIONS_OFF);
    _button_on      = _appModel->getGraphicTex(kGFX_BUTTON_ON);
    _button_off     = _appModel->getGraphicTex(kGFX_BUTTON_OFF);
    _button_deny    = _appModel->getGraphicTex(kGFX_BUTTON_DENY);
    _meter_on       = _appModel->getGraphicTex(kGFX_ARROWS_ON);
    _meter_off      = _appModel->getGraphicTex(kGFX_ARROWS_OFF);

    /********************************************************
     *      setup meter FBO and Shader                      *
     ********************************************************/

    // allocate fbo's
#if OF_VERSION < 7
    _meterMaskTex.allocate(147, 225, GL_RGBA); // size of the meter_on/off png
    _meterMaskFBO.setup(147, 225);
    _meterMaskFBO.attach(_meterMaskTex);

    _stationMaskTex.allocate(147, 515, GL_RGBA); // size of the meter_on/off png
    _stationMaskFBO.setup(147, 515);
    _stationMaskFBO.attach(_stationMaskTex);
#else
    _meterMaskFBO.allocate(147, 225);
    _stationMaskFBO.allocate(147, 515);
#endif
}

void AttackView::update() {

    _scaledInputLevel           = (float)_appModel->getARDArea(); //(float)_appModel->getPinInput()[0]/600.0f;      // make more complex soon ;-)

    BaseMeterView::update(kINTERACTION_ATTACKER);

}
