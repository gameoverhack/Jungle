#ifndef H_SOUNDCONTROLLER
#define H_SOUNDCONTROLLER

#include "AppModel.h"
#include "Fader.h"

typedef struct {
    //Fader* fader;
    string sequenceName;
    int startFrame;
    int endFrame;
    int duration;
    FaderType type;
    bool done;
} jungle_fade;

class SoundController {

public:

    SoundController();
    ~SoundController();

    void    setup();
    void    update();
    void    loadSound();

    void    fade(float toVolume, int timeMillis, FaderType fadeType);

    void    setVolume(float volume);

private:

    Fader*              _currentFade;

    ofSoundPlayer       _player;

    vector<jungle_fade*>  _fades;
    int                   _fadeIndex;

};

#endif // H_SOUNDCONTROLLER
