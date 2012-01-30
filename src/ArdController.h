/*
 *  ArdController.h
 *  Jungle
 *
 *  Created by gameover on 29/07/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_ARDCONTROLLER
#define _H_ARDCONTROLLER

#include "ofMain.h"
#include "BaseState.h"
#include "AppModel.h"

enum {
    kARDCONTROLLER_RETARD,
    kARDCONTROLLER_DISABLED,
	kARDCONTROLLER_INIT,
	kARDCONTROLLER_READY,
};

class ArdController : public BaseState {

public:

    ArdController(string deviceName, int ardBufferLengthSecs);
    ~ArdController();

	void	        registerStates();

    void            update();

    void            fakeAttackAction(float input);

private:

    void        setupArduino();
    void        updateArduino(bool fake = false);

    bool        _bSetupArduino;

    ofArduino   _ard;

    int         _bufferIntervalMillis;
    int         _lastUpdateTime;

    int         _ardCyclicBufferSize;
    int         _ardCyclicBufferOffset;
    int         _ardLastCyclicBufferOffset;

};

#endif
