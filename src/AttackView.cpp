#include "AttackView.h"

AttackView::AttackView(float width, float height) : BaseMeterView(width, height) {

    LOG_NOTICE("Setting up AttackView");

    /********************************************************
     *      Set unique x y co ordinates for assets          *
     ********************************************************/

    _bar_x                 = 102.0f;
    _icon_x                = 20.0f;
    _icon_y                = 700.0f;
    _meter_x               = 92.0f;
    _meter_y               = 10.0f;

    /********************************************************
     *      Get _appModel objects for drawing               *
     ********************************************************/

    _icon_on       = _appModel->getGraphicTex(kGFX_PUNCH_ON);
    _icon_off      = _appModel->getGraphicTex(kGFX_PUNCH_OFF);
    _icon_bar      = _appModel->getGraphicTex(kGFX_PUNCH_BAR);

}

void AttackView::update() {

    _scaledInputLevel           = (float)_appModel->getARDArea(); //(float)_appModel->getPinInput()[0]/600.0f;      // make more complex soon ;-)

    BaseMeterView::update(kINTERACTION_ATTACKER);

}
