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

//    if (_appModel->_timeAtPeak != -1 && (ofGetElapsedTimeMillis() - _appModel->_timeAtPeak < 8000)) {
//        _scaledInputLevel = 1.0;
//    } else {
//        _scaledInputLevel = ((float)_appModel->getFFTArea());
//        _appModel->_timeAtPeak = -1;
//    }

    _scaledInputLevel = ((float)_appModel->getFFTArea());

    /********************************************************
     *      Get _appModel Interactivity & Threshold         *
     ********************************************************/

    int   currentInteractivity  = _appModel->getCurrentInteractivity();

    int   totalNumSequences     = _appModel->getCurrentScene()->getNumOfSequences();
    int   thisNumSequence       = _appModel->getCurrentSequence()->getNumber();

    bool  deny                  = (ofGetElapsedTimeMillis() - _appModel->getLastActionTime() > TIMEOUT_ACTION);
    bool  isInteractive         = (currentInteractivity == kINTERACTION_BOTH || currentInteractivity == kINTERACTION_VICTIM);

    /********************************************************
     *      Draw the Meter to the ViewFBO                   *
     ********************************************************/

    drawMeterMask(_scaledInputLevel, _meterSteps, _meterPixelsForStep, &_meterMaskFBO);

    _viewFBO.begin();

    glPushMatrix();
    ofEnableAlphaBlending();

    glClearColor(0.0, 0.0, 0.0, 1.0); // black background no transparency
    glClear(GL_COLOR_BUFFER_BIT);

    int sceneCurrentFrame   = _appModel->getCurrentSceneFrame();
    int sceneTotalFrames    = _appModel->getCurrentSceneNumFrames();
    float turtlePosY        = 500 * ((float)sceneCurrentFrame/(float)sceneTotalFrames); // 500 distance between T intersections on turtle_bar

    _turtle_bar->draw(_turtle_bar_x, _turtle_bar_y);
    _turtle->draw(_turtle_x, _turtle_y - turtlePosY);

    _top_off->draw(_top_x, _top_y);

    if (isInteractive && _scaledInputLevel >= 0.99f) {
        _top_on->draw(_top_x, _top_y);
    } else if (!isInteractive && _scaledInputLevel > 0.05f) {
        _top_off->draw(_top_x, _top_y);
        if (deny) _top_deny->draw(_top_x, _top_y);
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
