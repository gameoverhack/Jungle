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

#define MAX_ARD_PINS 2

#include "BaseState.h"
#include "Singleton.h"
#include "AppDataTypes.h"

#include <boost/any.hpp>

#include <map>
#include <vector>
#include <assert.h>
#include <string>

using std::map;
using std::vector;
using std::string;

enum {
	kAPP_INIT,
	kAPP_LOADING,
	kAPP_RUNNING,
	kAPP_EXITING
};

class AppModel : public BaseState {

public:

	AppModel();
	~AppModel();

	void		        registerStates();

	void		        setScene(string name, Scene * scene);
	bool		        setCurrentScene(string sceneName);

	Sequence		    *getCurrentSequence();
	Scene			    *getCurrentScene();

	Scene			    *getScene(string sceneName);

    void                setARDRawPinState(int pin, int val);
    int                 getARDRawPinState(int pin);

	void		        setCameraTextures(ofTexture * victimCamTex, ofTexture * attackCamTex);
	ofTexture *         getVictimCamTexRef();
	ofTexture *         getAttackCamTexRef();

	bool		        nextScene();

    bool                hasProperty(string propName);
	void		        setProperty(string propName, boost::any propVal);

	void		        getProperty(string propName, int & propVal);
	void		        getProperty(string propName, float & propVal);
	void		        getProperty(string propName, string & propVal);

	boost::any	        getProperty(string propName);

	string		        getAllPropsAsList();
	map<string, string> getAllPropsNameTypeAsMap();

	goThreadedVideo *	getCurrentVideoPlayer();
	goThreadedVideo *	getNextVideoPlayer();
	void				toggleVideoPlayers();

	bool				checkCurrentInteractivity(interaction_t interactionType);

	void				setCurrentFrame(int frame);
	int					getCurrentFrame();
	int					getCurrentFrameTotal();

	void				setCurrentIsFrameNew(bool isFrameNew);
	bool				getCurrentIsFrameNew();

private:

	inline string	    pad(string & objectName);
	int				    _padLength;

	bool			    is_int(const boost::any & operand);
	bool			    is_float(const boost::any & operand);
	bool			    is_string(const boost::any & operand);
	bool			    is_char_ptr(const boost::any & operand);
	bool			    is_bool(const boost::any & operand);

	map<string, boost::any>		_anyProps;

	map<string, Scene *>		_scenes;

	Scene *						_currentScene;

	ofTexture *					_victimCamTex;
	ofTexture *					_attackCamTex;

	goThreadedVideo *			_videoPlayers[2];
	int							_frame;
	bool						_isFrameNew;

    int                         _ardRawPins[MAX_ARD_PINS];

protected:

};

typedef Singleton<AppModel> AppModelSingleton;					// Global declaration

static AppModel * _appModel	= AppModelSingleton::Instance();	// this is my idea of a compromise:
																// no defines, keeps introspection
																// but allows data sharing between classes

#endif

