/*
 *  HighLowTimer.h
 *
 */


#ifndef	_HIGHLOWTIMER_H
#define _HIGHLOWTIMER_H

#include "ofMain.h"
#include "ofThread.h"
#include "ofEvents.h"
#include "Timer.h"

/*

 TODO: This documentation is slightly out of date, especially regarding events

 Timer that will alternate between two values for a set number of cycles (or infinitely)

 The timer begins with "high".

 Events are triggered when timers begin, so the first event (with the high value)
 is sent as soon as you hit start(). Once the high timer is finished, the "value"
 changes to Low, the HighLowValueChanged is triggered with the low value, and the
 low timer is started.

 There are two kinds of events:
	-	HighLowValueChanged, before each timer is about to run,
		with the corresponding high or low value included.
	-	HighLowTimerFinished, fired when the requested number of cycles is complete

 Usage:

 HighLowTimer hlt()

 // High timer for 4 seconds, high value of 1.0, low timer for 2 seconds, low value of 0.0
 // repeat infinitely, timer resolution of 51 milliseconds
 hlt.setup(4000, 1.0, 2000, 0.0)
 // same as hlt.setup(4000, 1.0, 2000, 0.0, HighLowTimer::INFINITE, 51_)
 ofAddListener(hlt.HighLowValueChanged, this, MyClass::MyValueChangedMethod);
 ofAddListener(hlt.HighLowTimerFinished, this, MyClass::HighLowTimerHasFinishedMethod);
 hlt.start();

 */

enum HighLowTimerEventType{
	HIGHLOWTIMER_EVENT_START = 0,
	HIGHLOWTIMER_EVENT_COMPLETE,
	HIGHLOWTIMER_EVENT_RESTART,
	HIGHLOWTIMER_EVENT_STOP,
	HIGHLOWTIMER_EVENT_HIGH_START,
	HIGHLOWTIMER_EVENT_HIGH_COMPLETE,
	HIGHLOWTIMER_EVENT_LOW_START,
	HIGHLOWTIMER_EVENT_LOW_COMPLETE,
	HIGHLOWTIMER_EVENT_FAIL,
    HIGHLOWTIMER_EVENT_UNKNOWN
};

enum HighLowTimerMode{
	HIGHLOWTIMER_MODE_CYCLECOUNT = 0,
    HIGHLOWTIMER_MODE_REPEAT,
    HIGHLOWTIMER_MODE_UNKNOWN
};

class HighLowTimerEvent;
class HighLowTimer {

public:

	static const int REPEAT = -1;

	static inline string getHighLowTimerModeAsString(HighLowTimerMode mode){
        switch (mode) {
            case HIGHLOWTIMER_MODE_CYCLECOUNT:
                return "HIGHLOWTIMER_MODE_CYCLECOUNT";
                break;
            case HIGHLOWTIMER_MODE_REPEAT:
                return "HIGHLOWTIMER_MODE_REPEAT";
                break;
            default:
                return "HIGHLOWTIMER_MODE_UNKNOWN";
                break;
        }
    };

    static inline string getHighLowTimerEventTypeAsString(HighLowTimerEventType type){
        switch (type) {
			case HIGHLOWTIMER_EVENT_START:
				return "HIGHLOWTIMER_EVENT_START";
				break;
			case HIGHLOWTIMER_EVENT_COMPLETE:
				return "HIGHLOWTIMER_EVENT_COMPLETE";
				break;
			case HIGHLOWTIMER_EVENT_RESTART:
				return "HIGHLOWTIMER_EVENT_RESTART";
				break;
			case HIGHLOWTIMER_EVENT_STOP:
				return "HIGHLOWTIMER_EVENT_STOP";
				break;
			case HIGHLOWTIMER_EVENT_HIGH_START:
				return "HIGHLOWTIMER_EVENT_HIGH_START";
				break;
			case HIGHLOWTIMER_EVENT_HIGH_COMPLETE:
				return "HIGHLOWTIMER_EVENT_HIGH_COMPLETE";
				break;
			case HIGHLOWTIMER_EVENT_LOW_START:
				return "HIGHLOWTIMER_EVENT_LOW_START";
				break;
			case HIGHLOWTIMER_EVENT_LOW_COMPLETE:
				return "HIGHLOWTIMER_EVENT_LOW_COMPLETE";
				break;
			case HIGHLOWTIMER_EVENT_FAIL:
				return "HIGHLOWTIMER_EVENT_FAIL";
				break;
			case HIGHLOWTIMER_EVENT_UNKNOWN:
				return "HIGHLOWTIMER_EVENT_UNKNOWN";
				break;
            default:
                return "HIGHLOWTIMER_EVENT_UNKNOWN";
                break;
        }
    };

	HighLowTimer();
	~HighLowTimer();

	void setup(int highLengthInMilliseconds, float highValue,
			   int lowLengthInMilliseconds, float lowValue,
			   HighLowTimerMode mode = HIGHLOWTIMER_MODE_REPEAT,
			   string timerName = "UnnamedHighLowTimer",
			   int timerResolution = 51);

	void setup(int highLengthInMilliseconds, float highValue,
			   int lowLengthInMilliseconds, float lowValue,
			   int cycleLimit = 1,
			   string timerName = "UnnamedHighLowTimer",
			   int timerResolution = 51);

	bool start();
	bool restart();
	bool stop();

	void highTimerEventCallback(const void *sender, TimerEvent &event);
	void lowTimerEventCallback(const void *sender, TimerEvent &event);

	bool isTimerRunning();
	bool isHigh();
	bool isLow();
	string getName();
	void setName(string name);

	ofEvent<HighLowTimerEvent> highLowTimerEvent;

private:

	bool start(bool suppressEvents);
	bool stop(bool suppressEvents);

	Timer	_highTimer;
	Timer	_lowTimer;

    bool _high;
    bool _low;

	int	_highLength;
	int _lowLength;
	float _highValue;
	float _lowValue;
	int _cycleLimit;
	int _cycleCount;

	HighLowTimerMode _mode;

	int _timerResolution;

	string _name;
};

class HighLowTimerEvent{

public:

	HighLowTimerEvent(string name, HighLowTimer * timer, HighLowTimerEventType type):
    _name(name), _value(0.0), _timer(timer), _type(type){};

    HighLowTimerEvent(string name, float value, HighLowTimer * timer, HighLowTimerEventType type):
    _name(name), _value(value), _timer(timer), _type(type){};

    string _name;
	float _value;
    HighLowTimer * _timer;
    HighLowTimerEventType _type;
};



#endif



