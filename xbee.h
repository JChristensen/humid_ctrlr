#ifndef _xb_H
#define _xb_H
#include <gsXBee.h>                    //http://github.com/JChristensen/gsXBee
#include <Streaming.h>                 //http://arduiniana.org/libraries/streaming/
#include <Time.h>                      //http://www.arduino.cc/playground/Code/Time
#include <Timezone.h>                  //http://github.com/JChristensen/Timezone

const uint32_t LATE_DATA_MS(31000);       //data considered stale if not received for this long
const uint32_t XB_ACK_TIMEOUT(3000);      //ms
const int _SYNC_MINUTE(58);               //the minute in which the hourly time sync is done
const time_t _SYNC_INTERVAL(60*60);       //normal time sync interval
const time_t _SYNC_RETRY_INTERVAL(5*60);  //interval to retry time sync if not received

enum xbeeStates_t { xb_WAIT, xb_REQ_TIMESYNC, xb_WAIT_ACK };
enum timeTypes_t { UTC, LOCAL };

/*----------------------------------------------------------------------*
 * xb class                                                             *
 *----------------------------------------------------------------------*/
class xb : public gsXBee
{
public:
    xb();
    bool begin(Stream &serial, bool resetXBee = true);
    bool run(void);
    int temperature(void) { return _tF10; }
    bool dataStale(void) { return _stale; }

    void processTimeSync(time_t t);
    time_t lastTimeSync(void) { return _lastTimeSyncRecd; }
    time_t nextTimeSync(void) { return _nextTimeSync; }
    time_t timeNow(timeTypes_t type = UTC);
    void printDateTime(timeTypes_t type = UTC);
    void printDateTime(time_t t, timeTypes_t type = UTC);
    void printDate(time_t t);
    void printTime(time_t t);
    void printI00(int val, char delim);

private:
    xbeeStates_t _xbeeState;
    int _tF10;                  //plenum temperature, F * 10
    uint32_t _msLastData;       //last time data was received
    bool _stale;                //false if data is current

    time_t _utc;                //current utc time
    time_t _local;              //current local time
    time_t _utcStart;           //sketch start time (actually the first time sync received)
    time_t _lastTimeSyncRecd;   //last time sync received
    time_t _nextTimeSync;       //time for next time sync
    time_t _timeSyncRetry;      //used for retries

};

#endif
