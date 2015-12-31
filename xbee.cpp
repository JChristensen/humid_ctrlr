#include "xbee.h"

const uint32_t LATE_DATA_MS(31000);                     //data considered stale if not received for this long
const uint32_t XB_ACK_TIMEOUT(3000);                    //ms

xb::xb()
{
    _stale = true;
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
