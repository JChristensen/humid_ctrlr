//Issue: Humidifier doesn't turn off until one sample after falling below tOFF.

#include <LiquidTWI.h>                 //http://forums.adafruit.com/viewtopic.php?t=21586
// or http://dl.dropboxusercontent.com/u/35284720/postfiles/LiquidTWI-1.5.1.zip
#include <gsXBee.h>                    //http://github.com/JChristensen/gsXBee
#include <Streaming.h>                 //http://arduiniana.org/libraries/streaming/
#include <Time.h>                      //http://www.arduino.cc/playground/Code/Time
#include <Timezone.h>                  //http://github.com/JChristensen/Timezone
#include <Wire.h>
#include <XBee.h>                      //http://code.google.com/p/xbee-arduino/
#include "classes.h"
#include "clock.h"
#include "xbee.h"

//pin assignments
const uint8_t RELAY(7);
const uint8_t HB_LED(11);               //heartbeat
const uint8_t HUMID_ON_LED(12);         //humidifier on
const uint8_t DATA_STALE_LED(13);

//other constants
const uint32_t RESET_DELAY(10000);

//object instantiations
HumidifierController Humidifier(RELAY, HUMID_ON_LED, DATA_STALE_LED);
xb XB;
clock Clock;
LiquidTWI LCD(0); //i2c address 0 (0x20)
heartbeat hbLED(HB_LED, 1000);

//global variables

void circuitTest()
{
    const uint32_t INTERVAL(2000);

    pinMode(RELAY, OUTPUT);
    pinMode(HB_LED, OUTPUT);
    pinMode(HUMID_ON_LED, OUTPUT);
    hbLED.begin();
    LCD.clear();
    LCD << F("Test mode");

    static uint32_t msLast = millis();
    while (1)
    {
        hbLED.run();
        static bool state;
        static uint16_t count;
        if ( millis() - msLast >= INTERVAL )
        {
            msLast += INTERVAL;
            state = !state;
            if (state) ++count;
            digitalWrite(RELAY, state);
            digitalWrite(HUMID_ON_LED, state);
            LCD.setCursor(0, 1);
            if (state)
                LCD << F("ON ");
            else
                LCD << F("OFF");
            LCD << ' ' << count;
        }
    }
}

enum INIT_STATES_t {
    INIT_HARDWARE, REQ_TIMESYNC, WAIT_TIMESYNC, INIT_COMPLETE };

void setup(void)
{
    //special LCD characters
    uint8_t upArrow[8] = { B00100, B01110, B10101, B00100, B00100, B00100, B00100 };
    uint8_t dnArrow[8] = { B00100, B00100, B00100, B00100, B10101, B01110, B00100 };

    INIT_STATES_t INIT_STATE = INIT_HARDWARE;

    while ( INIT_STATE != INIT_COMPLETE )
    {
        switch (INIT_STATE)
        {
        case INIT_HARDWARE:
            Serial.begin(115200);
            Serial << endl << millis() << F( "\t" __FILE__ " " __DATE__ " " __TIME__ "\n" );
            LCD.begin(16, 2);
            LCD.createChar(0, upArrow);
            LCD.createChar(1, dnArrow);
            LCD.clear();
            LCD << F(__FILE__);
            delay(1000);
            LCD.clear();
            LCD << F(__DATE__);
            LCD.setCursor(0, 1);
            LCD << F(__TIME__);
            delay(1000);
            LCD.clear();
            LCD << F("XBee init");
            if ( !XB.begin(Serial) )
            {
                XB.mcuReset(RESET_DELAY);    //reset if XBee initialization fails
                //    if ( !XB.begin(Serial) ) circuitTest();
            }
            else
            {
                INIT_STATE = REQ_TIMESYNC;
                XB.setSyncCallback(clockSync);
            }
            break;

        case REQ_TIMESYNC:
            INIT_STATE = WAIT_TIMESYNC;
            LCD.clear();
            LCD << F("Time sync");
            XB.requestTimeSync(Clock.utc());
            break;

        case WAIT_TIMESYNC:
            XB.run();
            if ( Clock.lastTimeSync() > 0 )
            {
                INIT_STATE = INIT_COMPLETE;
                Serial << millis() << F("\tTime sync\t");
                Clock.printDateTime();
                Serial << endl;
                LCD.clear();
                Humidifier.begin();
                hbLED.begin();
            }
            break;

        case INIT_COMPLETE:
            break;
        }
    }
}

void loop(void)
{
    hbLED.run();
    bool haveData = XB.run();
    int t = XB.temperature();
    bool stale = XB.dataStale();
    static bool lastStale;
    static ctrlrStates_t hState;                            //humidifier state
    static ctrlrStates_t last_hState = (ctrlrStates_t)99;   //prev state (initialize to force update first time)

    //run the humidifier state machine if new data received or if the stale attribute has changed
    if ( haveData || stale != lastStale )
    {
        hState = Humidifier.run(t, stale);
        Serial << millis() << '\t';
        Clock.printDateTime();
        Serial << '\t' << t << '\t' << stale << '\t' << hState << endl;
    }

    //update the display if new data, stale attribute changed, or humidifier state changed
    if ( haveData || stale != lastStale || hState != last_hState )
    {
        LCD.clear();
        LCD << F("Humidifier ");
        LCD.setCursor(11, 0);
        if ( hState == H_ON_INCR || hState == H_ON_DECR )
            LCD << F("ON ");
        else
            LCD << F("OFF");

        LCD.setCursor(0, 1);
        LCD << t / 10 << '.' << t % 10 << '\xDF' << F("F ");
        if ( hState == H_STALE || hState == H_IDLE )
            LCD << '?';
        else if ( hState == H_OFF )
            LCD << ' ';
        else if ( hState == H_ON_INCR )
            LCD << '\x00';
        else if ( hState == H_ON_DECR || H_OFF_DECR )
            LCD << '\x01';
        LCD << F("  ");
    }

    //update saved values
    lastStale = stale;
    last_hState = hState;
}

void clockSync(time_t t)
{
    Clock.processTimeSync(t);
}