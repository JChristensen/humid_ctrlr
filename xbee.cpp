#include "xbee.h"
#include "clock.h"

xb::xb()
{
    _stale = true;
}

//initialize the XBee and give it the address of the clock object.
bool xb::begin(Stream &serial, clock *Clock, bool resetXBee)
{
    _Clock = Clock;
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
        if ( _Clock->lastTimeSync() > 0 )   //wait for setup to do the first sync
        {
            if ( _Clock->utc() >= _Clock->_nextTimeSync +  _Clock->_timeSyncRetry ) _xbeeState = xb_REQ_TIMESYNC;
        }
        break;
        
    case xb_REQ_TIMESYNC:
        _Clock->_timeSyncRetry += _SYNC_RETRY_INTERVAL;
        //xb.destAddr = coordinator;
        requestTimeSync(_Clock->utc());
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
