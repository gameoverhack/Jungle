#include "AttackView.h"

AttackView::AttackView(float width, float height) : BaseView(width, height) {
	// nothing?
}

void AttackView::update() {

    ofTexture   * meter_on      = _appModel->getGraphicTex(kGFX_METER_ON);
    ofTexture   * meter_off     = _appModel->getGraphicTex(kGFX_METER_OFF);
    ofTexture   * meter_level   = _appModel->getGraphicTex(kGFX_METER_LEVEL);
    ofTexture   * icon_on       = _appModel->getGraphicTex(kGFX_PUNCH_ON);
    ofTexture   * icon_off      = _appModel->getGraphicTex(kGFX_PUNCH_OFF);
    ofTexture   * icon_bar      = _appModel->getGraphicTex(kGFX_PUNCH_BAR);

    int currentInteractivity    = _appModel->getCurrentInteractivity();
    bool isInteractive          = (currentInteractivity == kINTERACTION_BOTH || currentInteractivity == kINTERACTION_ATTACKER);

    _viewFBO.begin();

    glClearColor(0.0, 0.0, 0.0, 0.0); // transparent clear colour
    glClear(GL_COLOR_BUFFER_BIT);

	ofSetColor(255, 255, 255, 255); // no tinting

    glPushMatrix();

    meter_off->draw(92.0, 10.0);

    if (isInteractive) {
        icon_on->draw(20.0, 700.0);
    } else {
        icon_off->draw(20.0, 700.0);
        icon_bar->draw(20.0, 700.0);
    }

    glPopMatrix();

    _viewFBO.end();

}

