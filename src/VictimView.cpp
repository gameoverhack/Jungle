#include "VictimView.h"

VictimView::VictimView(float width, float height) : BaseView(width, height) {
	// nothing?
}

void VictimView::update() {

    ofTexture   * meter_on      = _appModel->getGraphicTex(kGFX_METER_ON);
    ofTexture   * meter_off     = _appModel->getGraphicTex(kGFX_METER_OFF);
    ofTexture   * meter_level   = _appModel->getGraphicTex(kGFX_METER_LEVEL);
    ofTexture   * icon_on       = _appModel->getGraphicTex(kGFX_SCREAM_ON);
    ofTexture   * icon_off      = _appModel->getGraphicTex(kGFX_SCREAM_OFF);
    ofTexture   * icon_bar      = _appModel->getGraphicTex(kGFX_SCREAM_BAR);

    int currentInteractivity    = _appModel->getCurrentInteractivity();
    bool isInteractive          = (currentInteractivity == kINTERACTION_BOTH || currentInteractivity == kINTERACTION_VICTIM);

    _viewFBO.begin();

    glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
    glClear(GL_COLOR_BUFFER_BIT);

	ofSetColor(255, 255, 255, 255); // no tinting

    glPushMatrix();

    meter_off->draw(5.0, 10.0);

    if (isInteractive) {
        icon_on->draw(5.0, 700.0);
    } else {
        icon_off->draw(5.0, 700.0);
        icon_bar->draw(5.0, 700.0);
    }

    glPopMatrix();

    _viewFBO.end();

}
