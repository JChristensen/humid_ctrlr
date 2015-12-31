#ifndef _xb_H
#define _xb_H
#include <gsXBee.h>                    //http://github.com/JChristensen/gsXBee

class clock;

const uint32_t LATE_DATA_MS(31000);                     //data considered stale if not received for this long
const uint32_t XB_ACK_TIMEOUT(3000);                    //ms

enum xbeeStates_t { xb_WAIT, xb_REQ_TIMESYNC, xb_WAIT_ACK };

/*----------------------------------------------------------------------*
 * xb class                                                             *
 *----------------------------------------------------------------------*/
class xb : public gsXBee
{
public:
    xb();
    bool begin(Stream &serial, clock *Clock, bool resetXBee = true);
    bool run(void);
    int temperature(void) { return _tF10; }
    bool dataStale(void) { return _stale; }

private:
    clock* _Clock;
    xbeeStates_t _xbeeState;
    int _tF10;                  //plenum temperature, F * 10
    uint32_t _msLastData;       //last time data was received
    bool _stale;                //false if data is current
};

#endif
