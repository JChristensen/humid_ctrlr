#include "clock.h"
#include "xbee.h"
TimeChangeRule _EDT = { "EDT", Second, Sun, Mar, 2, -240 }; //Daylight time = UTC - 4 hours
TimeChangeRule _EST = { "EST", First, Sun, Nov, 2, -300 };  //Standard time = UTC - 5 hours
Timezone _myTZ(_EDT, _EST);
TimeChangeRule *_tcr;                     //pointer to the time change rule, use to get TZ abbrev

//give the clock object the address of the xb object.
void clock::begin(xb *XB)
{
    _XB = XB;
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
    static bool first(true);
    
    setTime(t);
    _lastTimeSyncRecd = t;
    _timeSyncRetry = 0;
    if ( first )            //calculate time for the first time sync
    {
        first = false;
        _utcStart = t;
        tmElements_t tm;
        breakTime(t, tm);
        tm.Minute = _SYNC_MINUTE;
        tm.Second = _XB -> txSec;
        _nextTimeSync = makeTime(tm);
        if ( _nextTimeSync <= t ) _nextTimeSync += _SYNC_INTERVAL;
    }
    else                    //calculate for subsequent syncs
    {
        while ( _nextTimeSync <= t ) _nextTimeSync += _SYNC_INTERVAL;
    }
    Serial << millis() << F("\tNext time sync\t");
    printDate(_nextTimeSync); printTime(_nextTimeSync); Serial << F("UTC\n");
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
