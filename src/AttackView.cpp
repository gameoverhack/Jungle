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

//    if (_appModel->_timeAtPush != -1 && (ofGetElapsedTimeMillis() - _appModel->_timeAtPush < 800)) {
//        _scaledInputLevel = 1.0f;
//    } else {
//        _scaledInputLevel = ((float)_appModel->getARDArea());
//        _appModel->_timeAtPush = -1;
//    }

    _scaledInputLevel = ((float)_appModel->getARDArea());

    /********************************************************
     *      Get _appModel Interactivity & Threshold         *
     ********************************************************/

    int   currentInteractivity  = _appModel->getCurrentInteractivity();

    int   totalNumSequences     = _appModel->getCurrentScene()->getNumOfSequences();
    int   thisNumSequence       = _appModel->getCurrentSequence()->getNumber();

    bool  deny                  = (ofGetElapsedTimeMillis() - _appModel->getLastActionTime() > TIMEOUT_ACTION);
    bool  isInteractive         = (currentInteractivity == kINTERACTION_BOTH || currentInteractivity == kINTERACTION_ATTACKER);

    /********************************************************
     *      Draw the Meter to the ViewFBO                   *
     ********************************************************/

    drawMeterMask(_scaledInputLevel, _meterSteps, _meterPixelsForStep, &_meterMaskFBO);

    drawMeterMask(thisNumSequence - 1, _stationSteps, _stationPixelsForStep, &_stationMaskFBO);

    _viewFBO.begin();

    glPushMatrix();
    ofEnableAlphaBlending();

    glClearColor(0.0, 0.0, 0.0, 1.0); // black background no transparency
    glClear(GL_COLOR_BUFFER_BIT);


    _bird->draw(_bird_x, _bird_y);

#if OF_VERSION < 7
    drawMeterShader(_stations_x, _stations_y, &_stationMaskTex, _stations_on, _stations_off);
#else
    drawMeterShader(_stations_x, _stations_y, &_stationMaskFBO.getTextureReference(), _stations_on, _stations_off);
#endif
    _button_off->draw(_button_x, _button_y);

    if (isInteractive && _scaledInputLevel > 0.05f) {
        _button_on->draw(_button_x, _button_y);
    } else if (!isInteractive && _scaledInputLevel > 0.05f) {
        _button_off->draw(_button_x, _button_y);
        if (deny) _button_deny->draw(_button_x, _button_y);
    }


    if (isInteractive) {
#if OF_VERSION < 7
        drawMeterShader(_meter_x, _meter_y, &_meterMaskTex, _meter_on, _meter_off);
#else
        drawMeterShader(_meter_x, _meter_y, &_meterMaskFBO.getTextureReference(), _meter_on, _meter_off);
#endif
    } else {

        _meter_off->draw(_meter_x, _meter_y);

    }

    ofDisableAlphaBlending();
    glPopMatrix();

    _viewFBO.end();

}
