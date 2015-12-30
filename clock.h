#include <Time.h>                      //http://www.arduino.cc/playground/Code/Time
#include <Timezone.h>                  //http://github.com/JChristensen/Timezone

const int _SYNC_MINUTE(58);               //the minute in which the hourly time sync is done
const time_t _SYNC_INTERVAL(60*60);       //normal time sync interval
const time_t _SYNC_RETRY_INTERVAL(5*60);  //interval to retry time sync if not received
TimeChangeRule _EDT = { 
    "EDT", Second, Sun, Mar, 2, -240     };    //Daylight time = UTC - 4 hours
TimeChangeRule _EST = { 
    "EST", First, Sun, Nov, 2, -300      };    //Standard time = UTC - 5 hours
Timezone _myTZ(_EDT, _EST);
TimeChangeRule *_tcr;                     //pointer to the time change rule, use to get TZ abbrev

class clock
{
public:
    clock(void);
    static time_t utc(void);
    static time_t local(void);
    static void processTimeSync(time_t t);
    static time_t lastTimeSync(void);
    void printDateTime(void);
    void printDateTime(time_t t);
    void printDate(time_t t);
    void printTime(time_t t);
    void printI00(int val, char delim);

//private:
    static time_t _utc;                              //current utc time
    static time_t _local;                            //current local time
    static time_t _utcStart;                         //sketch start time (actually the first time sync received)
    static time_t _lastTimeSyncRecd;                 //last time sync received
    static time_t _nextTimeSync;                     //time for next time sync
    static time_t _timeSyncRetry;                    //used to retry 
};

time_t clock::_utc;
time_t clock::_local;
time_t clock::_utcStart;
time_t clock::_lastTimeSyncRecd;
time_t clock::_nextTimeSync;
time_t clock::_timeSyncRetry;

clock::clock(void)
{
}

time_t clock::utc(void)
{
    _utc = now();
    _local = _myTZ.toLocal(_utc, &_tcr);   //TZ adjustment
    return _utc;
}

time_t clock::local(void)
{
    _utc = now();
    _local = _myTZ.toLocal(_utc, &_tcr);   //TZ adjustment
    return _local;
}

//set time to value received from master clock
void clock::processTimeSync(time_t t)
{
    setTime(t);
    _lastTimeSyncRecd = t;
    while ( _nextTimeSync <= _utc ) _nextTimeSync += _SYNC_INTERVAL;
    _timeSyncRetry = 0;
}

time_t clock::lastTimeSync(void)
{
    return _lastTimeSyncRecd;
}

//print current local date and time to Serial
void clock::printDateTime(void)
{
    time_t t = local();
    printDate(t);
    printTime(t);
    Serial << _tcr -> abbrev;
}

//print given date and time to Serial
void clock::printDateTime(time_t t)
{
    printDate(t);
    printTime(t);
    Serial << _tcr -> abbrev << endl;
}

//print given date to Serial
void clock::printDate(time_t t)
{
    Serial << year(t) << '-';
    printI00(month(t), '-');
    printI00(day(t), ' ');
}

//print given time to Serial
void clock::printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void clock::printI00(int val, char delim)
{
    if (val < 10) Serial << '0';
    Serial << val << delim;
    return;
}




