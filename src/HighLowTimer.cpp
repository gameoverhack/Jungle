/*
 *  HighLowTimer.cpp
 *
 */

#include "HighLowTimer.h"


HighLowTimer::HighLowTimer(){
//	setup(0, 1.0, 0, 0.0);
}

HighLowTimer::~HighLowTimer(){
	ofRemoveListener(_highTimer.timerEvent, this, &HighLowTimer::highTimerEventCallback);
	ofRemoveListener(_lowTimer.timerEvent, this, &HighLowTimer::lowTimerEventCallback);
};


// use this for limited high-low cycle count, use other for REPEAT or ONCE modes
void HighLowTimer::setup(int highLengthInMilliseconds, float highValue,
		   int lowLengthInMilliseconds, float lowValue,
		   int cycleLimit,
		   string timerName,
		   int timerResolution){
	// setup our timer, save some stuff
	_highLength = highLengthInMilliseconds;
	_highValue = highValue;
	_lowLength = lowLengthInMilliseconds;
	_lowValue = lowValue;
	_mode = HIGHLOWTIMER_MODE_CYCLECOUNT;
	_cycleLimit = cycleLimit;
	_cycleCount = 0;
	_timerResolution = timerResolution;
	_name = timerName;

    _high = _low = false;

	// setup the timers but don't start them here.
	_highTimer.setTimeout(_highLength, TIMER_MODE_ONCE, _name + " High Timer");
	ofAddListener(_highTimer.timerEvent, this, &HighLowTimer::highTimerEventCallback);

	_lowTimer.setTimeout(_lowLength, TIMER_MODE_ONCE, _name + " Low Timer");
	ofAddListener(_lowTimer.timerEvent, this, &HighLowTimer::lowTimerEventCallback);
}


// Use this setup for REPEAT or ONCE modes, use others for a limited high-low cycle count
void HighLowTimer::setup(int highLengthInMilliseconds, float highValue,
						 int lowLengthInMilliseconds, float lowValue,
						 HighLowTimerMode mode,
						 string timerName,
						 int timerResolution){
	setup(highLengthInMilliseconds, highValue, lowLengthInMilliseconds, lowValue, -1, timerName, timerResolution);
	_mode = mode;
}

/* Public timer methods */

bool HighLowTimer::start(){
	return start(false); // do NOT suppress events
}

bool HighLowTimer::stop(){
	return stop(false); // do NOT suppress events
}

bool HighLowTimer::restart(){
	// start acts as restart
	bool didStart = start(true); // suppress events

	if(didStart){
		HighLowTimerEvent event = HighLowTimerEvent(_name, this, HIGHLOWTIMER_EVENT_START);
		ofNotifyEvent(highLowTimerEvent, event, this);
	}

	return didStart;
}

/* Private timer methods */
bool HighLowTimer::start(bool suppressEvents){
	// sanity check our values
//	if(_highLength <= 0){
//		cout << "HIGH LENGTH LESS THAN 0";
//	}
//	if(_lowLength <= 0){
//		cout << "LOW LENGTH LESS THAN 0";
//	}
//	if(_timerResolution <= 0){
//		cout << "TIMER RESOLUTION LESS THAN 0";
//	}
//	if(_cycleLimit <= 0){
//		cout << "CYCLE LIMIT LESS THAN 0";
//	}
//	if(_mode != HIGHLOWTIMER_MODE_REPEAT){
//		cout << "MODE IS NOT REPEAT";
//	}
//	if((_cycleLimit <= 0 && _mode != HIGHLOWTIMER_MODE_REPEAT)){
//		cout << "NESTED IS TRUE";
//	}
//	if(_mode == HIGHLOWTIMER_MODE_UNKNOWN){
//		cout << "MODE IS UNKNOWN";
//	}
//
	if(_highLength <= 0 || _lowLength <= 0 || _timerResolution <= 0 ||
	   (_cycleLimit <= 0 && _mode != HIGHLOWTIMER_MODE_REPEAT) ||
	   _mode == HIGHLOWTIMER_MODE_UNKNOWN){
		return false;
	}


	// stop lowTimer if if is running
	_lowTimer.stop(); // TODO: suppress events here

	// start with highTimer
	if(_highTimer.start()){
	    _high = true;
	    _low = false;
		HighLowTimerEvent event = HighLowTimerEvent(_highTimer.getName(), _highValue, this, HIGHLOWTIMER_EVENT_HIGH_START);
		ofNotifyEvent(highLowTimerEvent, event, this);
		// dispatch high low timer started
		event = HighLowTimerEvent(_name, this, HIGHLOWTIMER_EVENT_START);
		ofNotifyEvent(highLowTimerEvent, event, this);
		// start success
		return true;
	}
	else{
		// TODO: could notify start_fail but why bother, shoudl it be fail or start_fail? stop_fail also????
		return false;
	}
}

bool HighLowTimer::stop(bool suppressEvents){
	// stop our sub-timers
	bool success = _lowTimer.stop() && _highTimer.stop();
    _high = false;
    _low = false;
	if(success && !suppressEvents){
		// dispatch our stop event
		HighLowTimerEvent event = HighLowTimerEvent(_name, this, HIGHLOWTIMER_EVENT_STOP);
		ofNotifyEvent(highLowTimerEvent, event, this);
	}

	return success;
}

void HighLowTimer::highTimerEventCallback(const void *sender, TimerEvent &event){
	//cout << "HighLowTimer highTimer callback: " << event._name << " " << Timer::getTimerEventTypeAsString(event._type) << endl;
	switch(event._type){
		case TIMER_EVENT_COMPLETE:{
			// ALWAYS want to star the low timer after the high timer has finished
			// TODO: Need some way to package the value up with this
			_high = false;
            _low = true;

			HighLowTimerEvent event = HighLowTimerEvent(_highTimer.getName(), this, HIGHLOWTIMER_EVENT_HIGH_COMPLETE);
			ofNotifyEvent(highLowTimerEvent, event, this);

			if(_lowTimer.start()){
				// send our LOW_START event
				HighLowTimerEvent event = HighLowTimerEvent(_lowTimer.getName(), _lowValue, this, HIGHLOWTIMER_EVENT_LOW_START);
				ofNotifyEvent(highLowTimerEvent, event, this);
			}

			break;
		}
		default:
			// do nothing
			break;
	}
}

void HighLowTimer::lowTimerEventCallback(const void *sender, TimerEvent &event){
	//cout << "HighLowTimer lowTimer callback: " << event._name << " " << Timer::getTimerEventTypeAsString(event._type) << endl;
	switch(event._type){
		case TIMER_EVENT_COMPLETE:{

		    _high = true;
            _low = false;

			HighLowTimerEvent event = HighLowTimerEvent(_lowTimer.getName(), this, HIGHLOWTIMER_EVENT_LOW_COMPLETE);
			ofNotifyEvent(highLowTimerEvent, event, this);

			// have done a cycle
			_cycleCount++;
			if(_cycleCount < _cycleLimit || _mode == HIGHLOWTIMER_MODE_REPEAT){
				// we have more cycles to do
				if(_highTimer.start()){
					// notify high timer started
					HighLowTimerEvent event = HighLowTimerEvent(_highTimer.getName(), _highValue, this, HIGHLOWTIMER_EVENT_HIGH_START);
					ofNotifyEvent(highLowTimerEvent, event, this);
				}
			}else{
				// no more cycles, so send the complete event
				HighLowTimerEvent event = HighLowTimerEvent(_name, this, HIGHLOWTIMER_EVENT_COMPLETE);
				ofNotifyEvent(highLowTimerEvent, event, this);
				stop(true); // stop anything that might have been running (safey), suppress any events
			}
			break;
		}
		default:
			// do nothing
			break;
	}
}

bool HighLowTimer::isTimerRunning(){
	return (_highTimer.isTimerRunning() || _lowTimer.isTimerRunning());
}

bool HighLowTimer::isHigh(){
    return _high;
}

bool HighLowTimer::isLow(){
    return _low;
}

string HighLowTimer::getName(){
	return _name;
}

void HighLowTimer::setName(string name){
	_name = name;
}

