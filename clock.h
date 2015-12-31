#ifndef _clock_H
#define _clock_H
#include <Streaming.h>                 //http://arduiniana.org/libraries/streaming/
#include <Time.h>                      //http://www.arduino.cc/playground/Code/Time
#include <Timezone.h>                  //http://github.com/JChristensen/Timezone

class xb;

const int _SYNC_MINUTE(58);               //the minute in which the hourly time sync is done
const time_t _SYNC_INTERVAL(60*60);       //normal time sync interval
const time_t _SYNC_RETRY_INTERVAL(5*60);  //interval to retry time sync if not received

class clock
{
public:
    clock() {}
    void begin(xb *XB);
    time_t utc(void);
    time_t local(void);
    void processTimeSync(time_t t);
    time_t lastTimeSync(void) { return _lastTimeSyncRecd; }
    time_t nextTimeSync(void) { return _nextTimeSync; }
    void printDateTime(void);
    void printDateTime(time_t t);
    void printDate(time_t t);
    void printTime(time_t t);
    void printI00(int val, char delim);
    friend class xb;

private:
    xb* _XB;
    time_t _utc;                              //current utc time
    time_t _local;                            //current local time
    time_t _utcStart;                         //sketch start time (actually the first time sync received)
    time_t _lastTimeSyncRecd;                 //last time sync received
    time_t _nextTimeSync;                     //time for next time sync
    time_t _timeSyncRetry;                    //used for retries
#endif

};
