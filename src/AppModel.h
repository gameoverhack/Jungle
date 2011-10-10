/*
 *  AppModel.h
 *  Jungle
 *
 *  Created by gameover on 29/06/11.
 *  Copyright 2011 trace media. All rights reserved.
 *
 */

#ifndef _H_APPMODEL
#define	_H_APPMODEL

#include "BaseState.h"
#include "Singleton.h"
#include "AppDataTypes.h"

#include <boost/any.hpp>

#include <map>
#include <vector>
#include <assert.h>
#include <string>

//#define EXTENDED_DISPLAY

using std::map;
using std::vector;
using std::string;

enum {
	kAPP_INIT,
	kAPP_LOADING,
	kAPP_RUNNING,
	kAPP_EXITING
};

enum {
    kGFX_ARROWS_ON = 0,
    kGFX_ARROWS_OFF,
    kGFX_BUTTON_ON,
    kGFX_BUTTON_OFF,
    kGFX_BUTTON_DENY,
    kGFX_METER_ON,
    kGFX_METER_OFF,
    kGFX_TOP_ON,
    kGFX_TOP_OFF,
    kGFX_TOP_DENY,
    kGFX_STATIONS_ON,
    kGFX_STATIONS_OFF,
    kGFX_BIRD,
    kGFX_TURTLE,
    kGFX_TURTLE_BAR,
    kGFX_TOTAL
};

class AppModel : public BaseState {

public:

	AppModel();
	~AppModel();

	void		        registerStates();

    // scene and sequence getter/setters
	void		        setScene(string name, Scene * scene);
	bool		        setCurrentScene(string sceneName);

	map<string, Scene *> getScenes(){
		return _scenes;
	}

	void				clearScenesAndSequences();

	Sequence		    *getCurrentSequence();
	Scene			    *getCurrentScene();

	Scene			    *getScene(string sceneName);

    bool		        nextScene();

    //void                setCurrentSequenceLevel(); // just doing this internally for now with _currentSceneLevel
    float               getCurrentSequenceLevel();     // possibly need different levels for attack and victim, might need to pass type?

    // interactivity getter/setters
    bool				checkCurrentInteractivity(interaction_t interactionType);
    void                setCurrentInteractivity(int frame); // perhaps should be private???
    int                 getCurrentInteractivity();

    // arduino getter/setters
    void                allocatePinInput(int numPins);
    int *               getPinInput();

    void                setARDArea(float area);
    float               getARDArea();

    void                setARDCyclicBufferSize(int size);
    int                 getARDCyclicBufferSize();

    void                allocateARDCyclicBuffer(int ardCyclicBufferSize);
    void                allocateARDNoiseFloor();
    void                allocateARDCyclicSum();
    void                allocateARDPostFilter();

    float *             getARDCyclicBuffer();
    float *             getARDNoiseFloor();
    float *             getARDCyclicSum();
    float *             getARDPostFilter();

    // audio/mic getter/setters
    void                setFFTArea(float area);
    float               getFFTArea();

    void                setFFTBinSize(int size);
    int                 getFFTBinSize();

    void                setAudioBufferSize(int size);
    int                 getAudioBufferSize();

    void                setFFTCyclicBufferSize(int size);
    int                 getFFTCyclicBufferSize();

    void                allocateFFTCyclicBuffer(int fftCyclicBufferSize, int fftBinSize);
    void                allocateFFTNoiseFloor(int fftBinSize);
    void                allocateFFTCyclicSum(int fftBinSize);
    void                allocateFFTPostFilter(int fftBinSize);
    void                allocateFFTInput(int fftBinSize);
    void                allocateAudioInput(int bufferSize);

    fftBands *          getFFTCyclicBuffer();
    float *             getFFTNoiseFloor();
    float *             getFFTCyclicSum();
    float *             getFFTPostFilter();
    float *             getFFTInput();
    float *             getAudioInput();

    // camera getter/setters
	void		        setCameraTextures(ofTexture * victimCamTex, ofTexture * attackCamTex);
	ofTexture *         getVictimCamTexRef();
	ofTexture *         getAttackCamTexRef();
	ofTexture *         getFakeVictimCamTexRef();
    ofTexture *         getFakeAttackCamTexRef();

    goVideoPlayer *     getFakeVictimPlayer();
    goVideoPlayer *     getFakeAttackPlayer();

    void                setCameraAttributes(int which, PosRotScale * prs);
    void                setFakeAttributes(int which, PosRotScale * prs);
    PosRotScale *       getCameraAttributes(int which);
    PosRotScale *       getFakeAttributes(int which);

    // video player getter/setters
    goThreadedVideo *	getCurrentVideoPlayer();
	goThreadedVideo *	getNextVideoPlayer();
	void				toggleVideoPlayers(int forceFrame = 0);

	void				setCurrentSequenceFrame(int frame);
	int					getCurrentSequenceFrame();
	int					getCurrentSequenceNumFrames();
    int                 getCurrentSceneFrame();
    int                 getCurrentSceneNumFrames();


	void				setCurrentIsFrameNew(bool isFrameNew);
	bool				getCurrentIsFrameNew();

    // graphic asset getter/setters
    bool                loadGraphicAsset(string path, int type);
    ofTexture         * getGraphicTex(int type);

    // generic property getter/setters
    bool                hasProperty(string propName);
	void		        setProperty(string propName, boost::any propVal);

	void		        getProperty(string propName, int & propVal);
	void		        getProperty(string propName, float & propVal);
	void		        getProperty(string propName, string & propVal);

	boost::any	        getProperty(string propName);

	string		        getAllPropsAsList();
	map<string, string> getAllPropsNameTypeAsMap();

	void				printAllScenes();

    void                setFacePresent(int face, bool isPresent) {_isFacePresent[face] = isPresent;};
    bool                getFacePresent(int face) {return _isFacePresent[face];};
    bool                getAnyFacePresent() {return _isFacePresent[0] || _isFacePresent[1];};

private:

	inline string	    pad(string & objectName);
	int				    _padLength;

	bool			    is_int(const boost::any & operand);
	bool			    is_float(const boost::any & operand);
	bool			    is_string(const boost::any & operand);
	bool			    is_char_ptr(const boost::any & operand);
	bool			    is_bool(const boost::any & operand);

	map<string, boost::any>		_anyProps;

    // scene and sequence vars
	map<string, Scene *>		_scenes;
	Scene *						_currentScene;

    // camera vars
	ofTexture *					_victimCamTex;
	ofTexture *					_attackCamTex;
	PosRotScale *               _cameraPRS[2];
	PosRotScale *               _fakePRS[2];

    // video vars
	goThreadedVideo *			_videoPlayers[2];
	goVideoPlayer *             _fakePlayers[2];
	int							_currentSequenceFrame;
	int                         _lastSequenceFrame;
	int                         _currentSceneFrame; // not inlcuding loops and bs
	bool						_isFrameNew;

    // arduino vars
    float                       _ardArea;

    int *                       _ardRawPins;

    float *                     _ardCyclicBuffer;
    int                         _ardCyclicBufferSize;

    float *                     _ardNoiseFloor;
    float *                     _ardCyclicSum;
	float *                     _ardPostFilter;

    // audio/fft vars
    float                       _fftArea;

    fftBands *                  _fftCyclicBuffer;

    float *                     _fftNoiseFloor;
	float *                     _fftCyclicSum;
	float *                     _fftPostFilter;
    float *                     _fftInput;
    int                         _fftBinSize;
    int                         _fftCyclicBufferSize;
    int                         _audioBufferSize;

	float *                     _audioInput;

    // graphic asset vars
    ofImage                     _gfxAssets[kGFX_TOTAL];

    // interactivity vars
    int                         _currentInteractivity;
    float                       _currentSequenceLevel;

    bool                        _isFacePresent[2];

};

typedef Singleton<AppModel> AppModelSingleton;					// Global declaration

static AppModel * _appModel	= AppModelSingleton::Instance();	// this is my idea of a compromise:
																// no defines, keeps introspection
																// but allows data sharing between classes

#endif

