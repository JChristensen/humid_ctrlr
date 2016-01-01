#include "xbee.h"

TimeChangeRule _EDT = { "EDT", Second, Sun, Mar, 2, -240 }; //Daylight time = UTC - 4 hours
TimeChangeRule _EST = { "EST", First, Sun, Nov, 2, -300 };  //Standard time = UTC - 5 hours
Timezone _myTZ(_EDT, _EST);
TimeChangeRule *_tcr;                     //pointer to the time change rule, use to get TZ abbrev

xb::xb()
{
    _stale = true;
}

//initialize the XBee
bool xb::begin(Stream &serial, bool resetXBee)
{
    gsXBee::begin(serial, resetXBee);
}

//returns true when data received
bool xb::run(void)
{
    bool ret = false;
    if ( millis() - _msLastData >= LATE_DATA_MS ) _stale = true;

    xbeeReadStatus_t xbStatus = read();         //check for incoming traffic
    if ( xbStatus == RX_DATA )
    {
        if ( payload[0] == 'S' )                //subtype S: sensor data
        {
            _msLastData = millis();
            _stale = false;
            ret = true;
            _tF10 = (uint8_t)payload[2] + ((uint8_t)payload[1] << 8);
        }
        else
        {
            Serial << millis() << F(" Unknown packet subtype '") << payload[0] << F("'\n");
        }
    }

    switch ( _xbeeState )
    {
    case xb_WAIT:
        if ( lastTimeSync() > 0 )   //wait for setup to do the first sync
        {
            if ( now() >= _nextTimeSync + _timeSyncRetry ) _xbeeState = xb_REQ_TIMESYNC;
        }
        break;

    case xb_REQ_TIMESYNC:
        _timeSyncRetry += _SYNC_RETRY_INTERVAL;
        //xb.destAddr = coordinator;
        requestTimeSync(now());
        _xbeeState = xb_WAIT_ACK;
        break;

    case xb_WAIT_ACK:
        if ( millis() - msTX >= XB_ACK_TIMEOUT )
        {
            _xbeeState = xb_WAIT;
        }
        else if (xbStatus == TX_ACK)
        {
            _xbeeState = xb_WAIT;
        }
        else if (xbStatus == TX_FAIL)
        {
            _xbeeState = xb_WAIT;
        }
        break;
    }

    return ret;
}

//set time to value received from master clock
void xb::processTimeSync(time_t t)
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
        tm.Second = txSec;
        _nextTimeSync = makeTime(tm);
        if ( _nextTimeSync <= t ) _nextTimeSync += _SYNC_INTERVAL;
    }
    else                    //calculate for subsequent syncs
    {
        while ( _nextTimeSync <= t ) _nextTimeSync += _SYNC_INTERVAL;
    }
    Serial << millis() << F("\tNext time sync\t");
    printDateTime(_myTZ.toLocal(_nextTimeSync, &_tcr), LOCAL); Serial << endl;
}

//return current time, UTC or local
time_t xb::timeNow(timeTypes_t type)
{
    if ( type == UTC )
        return now();
    else
        return _myTZ.toLocal(now(), &_tcr);
}

//print current date and time, UTC or local
void xb::printDateTime(timeTypes_t type)
{
    time_t t = now();
    if ( type == LOCAL ) t = _myTZ.toLocal(t, &_tcr);
    printDate(t);
    printTime(t);
    if ( type == LOCAL )
        Serial << _tcr -> abbrev;
    else
        Serial << F("UTC");
}

//print given date and time to Serial
void xb::printDateTime(time_t t, timeTypes_t type)
{
    printDate(t);
    printTime(t);
    if ( type == LOCAL )
        Serial << _tcr -> abbrev;
    else
        Serial << F("UTC");
}

//print given date to Serial
void xb::printDate(time_t t)
{
    Serial << year(t) << '-';
    printI00(month(t), '-');
    printI00(day(t), ' ');
}

//print given time to Serial
void xb::printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void xb::printI00(int val, char delim)
{
    if (val < 10) Serial << '0';
    Serial << val << delim;
    return;
}
