/*
 *  Timer.h
 *
 */

#ifndef	__TIMER_H
#define __TIMER_H

#include "ofMain.h"
#include "ofThread.h"
#include "ofEvents.h"

/*

 Generic Timer class

 Create a Timer, set a completion call back and hit start.

 Usage:

 Timer timer1()
 if(timer1.start() == false){
	// timer fails to start here because we have not used the setup constructor
	// or called setTimeout(...) before calling start()
 }

 // My Timer, fire once after 4 seconds
 timer1.setTimeout("My Timer", 4000)
 // same as timer1.setTimeout("My Timer", 4000, 51, Timer::ONCE)
 // or Timer timer1("My Timer", 4000, 51, Timer::ONCE)
 timer1.start()

 // My timer, fired every 3 seconds, checking to see
 // if we're over 3 seconds every 50 milliseconds
 timer1.setTimeout("My Timer", 3000, 50, Timer::REPEAT)
 timer1.start()

 void MyClass::MyMethod(const void *sender, string &name);
 ofAddListener(timer1.timerDidFinish, this, MyClass::MyMethod);
 // or timer1.hasTimedOut()
 // note: hasTimedOut() is not suitable for Timer::REPEAT timers,
 // because the hasTimedOut flag is reset after each timer "tick"
 // use the listener method instead.


 EVENT MODEL

 start() will trigger an TIMER_EVENT_START
 restart() will trigger a TIMER_EVENT_RESTART
 When a timer runs to completion it will trigger a TIMER_EVENT_COMPLETE,
 if the timer was set to TIMER_MODE_REPEAT, it will trigger TIMER_EVENT_RESTART
 stop() will trigger  a TIMER_EVENT_STOP.


 */

enum TimerEventType{
	TIMER_EVENT_START = 0,
    TIMER_EVENT_COMPLETE,
    TIMER_EVENT_RESTART,
    TIMER_EVENT_STOP,
    TIMER_EVENT_FAIL,
    TIMER_EVENT_UNKNOWN
};

enum TimerMode{
    TIMER_MODE_ONCE = 0,
    TIMER_MODE_REPEAT,
    TIMER_MODE_UNKNOWN
};

class TimerEvent;
class Timer : public ofThread {

public:

    static inline string getTimerModeAsString(TimerMode mode){
        switch (mode) {
            case TIMER_MODE_ONCE:
                return "TIMER_MODE_ONCE";
                break;
            case TIMER_MODE_REPEAT:
                return "TIMER_MODE_REPEAT";
                break;
            default:
                return "TIMER_MODE_UNKNOWN";
                break;
        }
    };

    static inline string getTimerEventTypeAsString(TimerEventType type){
        switch (type) {
			case TIMER_EVENT_START:
				return "TIMER_EVENT_START";
				break;
            case TIMER_EVENT_COMPLETE:
                return "TIMER_EVENT_COMPLETE";
                break;
            case TIMER_EVENT_RESTART:
                return "TIMER_EVENT_RESTART";
                break;
            case TIMER_EVENT_STOP:
                return "TIMER_EVENT_STOP";
                break;
            case TIMER_EVENT_FAIL:
                return "TIMER_EVENT_FAIL";
                break;
            default:
                return "TIMER_EVENT_UNKNOWN";
                break;
        }
    };

	Timer();

	Timer(int timeoutInMilliseconds,
          TimerMode mode = TIMER_MODE_ONCE,
          string timerName = "UnnamedTimer",
		  int resolutionInMilliseconds = 51);

	~Timer(){};

	void setTimeout(int timeoutInMilliseconds,
                    TimerMode mode = TIMER_MODE_ONCE,
                    string timerName = "UnnamedTimer",
                    int resolutionInMilliseconds = 51);

	bool start();
	bool restart();
	bool stop();

	bool		hasTimedOut();
	bool		isTimerRunning();
	void        setName(string timerName);
	string		getName();
	void		setVerbose(bool v);
	TimerMode 	getTimerMode();
	void		setTimerMode(TimerMode mode);
	float		getProgress();

	ofEvent<TimerEvent>	timerEvent;

private:

	void	checkTimeout();
	void	threadedFunction();

	bool	start(bool suppressEvents);
	bool	stop(bool suppressEvents);

	string      _name;
	int         _timeout;
	int         _timerResolution;
	TimerMode	_timerMode;
	bool        _hasTimedOut;
	int         _startTime;
	bool        _verbose;

    // block copy and assignment operators explicitly
    Timer(const Timer& other);
    Timer& operator=(const Timer&);

};

class TimerEvent{

public:

    TimerEvent(string name, Timer * timer, TimerEventType type):
    _name(name), _timer(timer), _type(type){};

    string _name;
    Timer * _timer;
    TimerEventType _type;
};

#endif

