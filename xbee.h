#ifndef _xb_H
#define _xb_H
#include <gsXBee.h>                    //http://github.com/JChristensen/gsXBee

enum xbeeStates_t { xb_WAIT, xb_WAIT_ACK };             //xb_WAIT_ACK not currently used

/*----------------------------------------------------------------------*
 * xb class                                                             *
 *----------------------------------------------------------------------*/
class xb : public gsXBee
{
public:
    xb();
    bool run(void);
    int temperature(void) { return _tF10; }
    bool dataStale(void) { return _stale; }

private:
    xbeeStates_t _xbeeState;
    int _tF10;                  //plenum temperature, F * 10
    uint32_t _msLastData;       //last time data was received
    bool _stale;                //false if data is current
};

#endif
