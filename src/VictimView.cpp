#include "VictimView.h"

VictimView::VictimView(float width, float height) : BaseMeterView(width, height) {

	LOG_NOTICE("Setting up VictimView");

    /********************************************************
     *      Set unique x y co ordinates for assets          *
     ********************************************************/

    _bar_x                 = 17.0f;
    _icon_x                = 5.0f;
    _icon_y                = 700.0f;
    _meter_x               = 5.0f;
    _meter_y               = 10.0f;

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    _icon_on       = _appModel->getGraphicTex(kGFX_SCREAM_ON);
    _icon_off      = _appModel->getGraphicTex(kGFX_SCREAM_OFF);
    _icon_bar      = _appModel->getGraphicTex(kGFX_SCREAM_BAR);

}

void VictimView::update() {

    _scaledInputLevel           = ((float)_appModel->getFFTArea());      // make more complex soon ;-)

    BaseMeterView::update(kINTERACTION_VICTIM);

}
