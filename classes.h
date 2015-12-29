//Humidifier controller class.
//run() should be called for each temperature reading, or if data stale attribute changes.

//controller set points F * 10
const int T_ON(800);         //turn on when plenum temperature exceeds this level
const int T_OFF(1000);       //turn off when temperature falls below this level

//enumerations
enum ctrlrStates_t { H_STALE, H_IDLE, H_OFF, H_ON_INCR, H_ON_DECR, H_OFF_DECR };
    
class HumidifierController
{
public:
    HumidifierController(uint8_t relayPin, uint8_t relayLedPin, uint8_t staleLedPin)
        : _relayPin(relayPin), _relayLedPin(relayLedPin), _staleLedPin(staleLedPin), _msOn(0), _msTotalOn(0), _onCount(0) {}
    void begin(void);
    ctrlrStates_t run(int temperature, bool dataStale);

private:
    ctrlrStates_t _CTL_STATE;
    uint8_t _relayPin;
    uint8_t _relayLedPin;   //LED pin turns on and off with relay
    uint8_t _staleLedPin;   //LED to indicate stale data
    uint32_t _msOn;         //time the humidifier was turned on (set to zero when humidifier off)
    uint32_t _msTotalOn;    //total on time for the humidifier
    uint16_t _onCount;      //number of times the humidifier was turned on
    int _prevTemperature;   //temperature on last pass through
};

void HumidifierController::begin(void)
{
    pinMode(_relayPin, OUTPUT);
    pinMode(_relayLedPin, OUTPUT);
    pinMode(_staleLedPin, OUTPUT);
}

ctrlrStates_t HumidifierController::run(int temperature, bool dataStale)
{
    if ( dataStale ) _CTL_STATE = H_STALE;
    
    switch ( _CTL_STATE )
    {
        case H_STALE:       //lost data feed, turn humidifier off
            _CTL_STATE = H_IDLE;
            if ( _msOn > 0 )
            {
                _msTotalOn += millis() - _msOn;
                _msOn = 0;
            }
            digitalWrite(_relayPin, LOW);
            digitalWrite(_relayLedPin, LOW);
            digitalWrite(_staleLedPin, HIGH);
            break;

        case H_IDLE:        //waiting for current data
            if ( !dataStale )
            {
                _CTL_STATE = H_OFF;
                digitalWrite(_staleLedPin, LOW);
            }
            break;
            
        case H_OFF:         //humidifier off
            if ( temperature >= T_ON )
            {
                _CTL_STATE = H_ON_INCR;
                _msOn = millis();
                ++_onCount;
                digitalWrite(_relayPin, HIGH);
                digitalWrite(_relayLedPin, HIGH);
            }
            break;
            
        case H_ON_INCR:     //humidifier on, temperature increasing
            if ( temperature < _prevTemperature )
            {
                _CTL_STATE = H_ON_DECR;
            }
            break;
            
        case H_ON_DECR:     //humidifier on, temperature decreasing
            if ( temperature <= T_OFF )
            {
                _CTL_STATE = H_OFF_DECR;
                _msTotalOn += millis() - _msOn;
                _msOn = 0;
                digitalWrite(_relayPin, LOW);
                digitalWrite(_relayLedPin, LOW);
            }                
            else if ( temperature > _prevTemperature )
            {
                _CTL_STATE = H_ON_INCR;
            }
            break;
            
        case H_OFF_DECR:    //humidifier off, cooling back down to T_ON
            if ( temperature <= T_ON )
            {
                _CTL_STATE = H_OFF;
            }
            break;
    }
    _prevTemperature = temperature;
    return _CTL_STATE;
}

class heartbeat
{
public:
    heartbeat(uint8_t pin, uint32_t interval) : _pin(pin), _interval(interval), _state(true) {}
    void begin(void);
    void run(void);

private:
    uint8_t _pin;
    uint32_t _interval;
    uint32_t _lastHB;
    bool _state;
};

void heartbeat::begin(void)
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, _state);
    _lastHB = millis();
}

void heartbeat::run(void)
{
    if ( millis() - _lastHB >= _interval )
    {
        _lastHB += _interval;
        digitalWrite( _pin, _state = !_state);
    }
}
