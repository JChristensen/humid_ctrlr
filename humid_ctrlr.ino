//Humidifier controller
//Jack Christensen Dec-2015
//Supplies power to humidifier when furnace is running by sensing
//temperature in the plenum.

#include <LiquidTWI.h>                 //http://forums.adafruit.com/viewtopic.php?t=21586
// or http://dl.dropboxusercontent.com/u/35284720/postfiles/LiquidTWI-1.5.1.zip
#include <gsXBee.h>                    //http://github.com/JChristensen/gsXBee
#include <Streaming.h>                 //http://arduiniana.org/libraries/streaming/
#include <Time.h>                      //http://www.arduino.cc/playground/Code/Time
#include <Timezone.h>                  //http://github.com/JChristensen/Timezone
#include <Wire.h>
#include <XBee.h>                      //http://code.google.com/p/xbee-arduino/
#include "classes.h"
#include "xbee.h"

const uint8_t               //pin assignments
//  RXD(0),
//  TXD(1),
    RELAY(7),
    HB_LED(11),             //heartbeat
    HUMID_ON_LED(12),       //humidifier on indicator
    DATA_STALE_LED(13),
//  SDA(A4),
//  SCL(A5),
    UNUSED_PINS[] = { 2, 3, 4, 5, 6, 8, 9, 10, A0, A1, A2, A3 };

//other constants
const uint32_t RESET_DELAY(10000);

//object instantiations
HumidifierController Humidifier(RELAY, HUMID_ON_LED, DATA_STALE_LED);
xb XB;
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
    INIT_STATES_t INIT_STATE = INIT_HARDWARE;

    while ( INIT_STATE != INIT_COMPLETE )
    {
        switch (INIT_STATE)
        {
        case INIT_HARDWARE:
            //enable pullups on unused pins for noise immunity
            for (uint8_t i = 0; i < sizeof(UNUSED_PINS) / sizeof(UNUSED_PINS[0]); i++) pinMode(i, INPUT_PULLUP);
            Serial.begin(115200);
            Serial << endl << millis() << F( "\t" __FILE__ " " __DATE__ " " __TIME__ "\n" );
            LCD.begin(16, 2);
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
            XB.requestTimeSync(now());
            break;

        case WAIT_TIMESYNC:
            XB.run();
            if ( XB.lastTimeSync() > 0 )
            {
                INIT_STATE = INIT_COMPLETE;
                Serial << millis() << F("\tTime sync\t");
                XB.printDateTime(LOCAL); Serial << endl;
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

    //run the humidifier state machine
    hState = Humidifier.run(t, stale);

    //update the display if new data, stale attribute changed, or humidifier state changed
    if ( haveData || stale != lastStale || hState != last_hState )
    {
        LCD.clear();
        LCD << F("Humidifier ");
        LCD.setCursor(11, 0);
        if ( hState == H_ON )
            LCD << F("ON ");
        else
            LCD << F("OFF");

        LCD.setCursor(0, 1);
        LCD << t / 10 << '.' << t % 10 << '\xDF' << F("F ");
        if ( hState == H_STALE || hState == H_IDLE )
            LCD << '?';
        LCD << F("  ");

        //print to Serial too
        Serial << millis() << '\t';
        XB.printDateTime(LOCAL);
        Serial << '\t' << t << '\t' << stale << '\t' << hState << endl;
    }

    //update saved values
    lastStale = stale;
    last_hState = hState;
}

void clockSync(time_t t)
{
    XB.processTimeSync(t);
}
