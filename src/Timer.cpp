/*
 *  Timer.cpp
 *
 */

#include "Timer.h"

/*
 * Constructors
 */

/*
 * Default Constructor
 *
 * You can not safely call start() before calling
 * setTimeout manually if using this constructor
 *
 */
Timer::Timer(){
	setVerbose(false);
	setTimeout(0); //setup with bad data
	_isRunning = false;
}

Timer::~Timer(){
    ofRemoveListener(ofEvents().update, this, &Timer::update);
}

/*
 * Full convenience constructor
 */
Timer::Timer(int timeoutInMilliseconds,
             TimerMode mode,
             string name,
             int resolutionInMilliseconds){
	setVerbose(false);
	setTimeout(timeoutInMilliseconds, mode, name, resolutionInMilliseconds);
}


/*
 * Public Timer methods
 */

/*
 * Setup timer
 */
void Timer::setTimeout(int timeoutInMilliseconds,
                       TimerMode mode,
                       string name,
                       int resolutionInMilliseconds){
	_timeout = timeoutInMilliseconds;
	_timerResolution = resolutionInMilliseconds;
	_timerMode = mode;
    _name = name;
    _hasTimedOut = false;
    _isRunning = false;
    ofAddListener(ofEvents().update, this, &Timer::update);
}

/*
 * Public Timer methods
 */


bool Timer::start(){
	start(false); // do NOT suppress events
}

bool Timer::stop(){
	stop(false); // do NOT suppress events
}

/*
 * Syntatic sugar, same as calling start()
 */
bool Timer::restart(){
	bool didStart = start(true); // suppress event

    // dispatch RESTART
	TimerEvent event = TimerEvent(_name, this, TIMER_EVENT_RESTART);
	ofNotifyEvent(timerEvent, event, this);

	return didStart; // start will clear state and start threads
}


/*
 * Private Timer methods
 */

/*
 * Start timing, returns false if impossible to start
 * Will restart current timer if required
 */
bool Timer::start(bool suppressEvents){
	// Create clean state
	_startTime = ofGetElapsedTimeMillis();
	_hasTimedOut = false;

	// sanity check that we are able to start
	if(_name == "" || _timeout <= 0 || _timerResolution <= 0){
		if(_verbose) cout << "Could not start timer " << _name << endl;
		return false;
	}

	//if(_verbose) cout << "Starting timer '" << _name << "' at time: " << ofToString(_startTime) << endl;
    //

	if(!suppressEvents){
		// dispatch START event
		TimerEvent event = TimerEvent(_name, this, TIMER_EVENT_START);
		ofNotifyEvent(timerEvent, event, this);
	}

     _isRunning = true;

	return true; // success
}

/*
 * Stop timing
 */
bool Timer::stop(bool suppressEvents){
    // we stopped prematurely
    if(!suppressEvents){
        //dispatch STOPPED
        TimerEvent event = TimerEvent(_name, this, TIMER_EVENT_STOP);
        ofNotifyEvent(timerEvent, event, this);
    }
    _isRunning = false;
	return true;
}


/*
 * Check if we have timed out, restart as required
 */
void Timer::update(ofEventArgs& e){
    if(!_isRunning) return;
	int timeSinceStart = ofGetElapsedTimeMillis() - _startTime;
	if(timeSinceStart >= _timeout){
		// Timeout is finished
		_hasTimedOut = true;
		if(_verbose) cout << "Timer " << _name << " timed out\n";

		// dispatch COMPLETE event, which will be
		// followed up with a RESTART event in restart()
        TimerEvent event = TimerEvent(_name, this, TIMER_EVENT_COMPLETE);
		ofNotifyEvent(timerEvent, event, this);

        switch (_timerMode) {
            case TIMER_MODE_REPEAT:
                // repeating, so start again
                cout << _name << " is set to restart..." << endl;
                restart();
                break;
            case TIMER_MODE_ONCE:
                // Not repeating, so stop the thread
                cout << _name << " stopping." << endl;
                stop(true); // supress stop event
                break;
            default:
                break;
        }
	}
}

/*
 * Boring getters/setters
 */

void Timer::setVerbose(bool v){
    //Poco::ScopedLock<ofMutex> lock();
	_verbose = v;
}

void Timer::setName(string timerName){
	_name = timerName;
}


string Timer::getName(){
	return _name;
}

/*
 *
 * Note: hasTimedOut() is useless when timerMode is Timer::REPEAT,
 * because hasTimedOut will be reset right away,
 * you will have to rely on the event firing.
 *
 */
bool Timer::hasTimedOut(){
    Poco::ScopedLock<ofMutex> lock();
	return _hasTimedOut;
}

bool Timer::isTimerRunning(){
	// timer is running if the thread is running and we have not timed out
	return _isRunning;
}


TimerMode Timer::getTimerMode(){
	return _timerMode;
}

void Timer::setTimerMode(TimerMode mode){
	Poco::ScopedLock<ofMutex> lock();
	_timerMode = mode;
}

float Timer::getProgress(){
	return isTimerRunning() ? (ofGetElapsedTimeMillis() - _startTime)/(float)(_timeout) : 0.0f;
}
