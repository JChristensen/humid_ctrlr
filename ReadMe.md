# Humidifier Controller #
http://github.com/JChristensen/humid_ctrlr  
ReadMe file  
Jack Christensen Jan 2016  

An Arduino-compatible controller that controls power to an Aprilaire humidifier. Installation of a high-efficiency furnace with a variable-speed fan created a problem. The humidifier is wired to run when the furnace fan runs. It was recommended that the furnace fan be set to run continuously, since the furnace will idle it down to a low speed when the burner isn't on. The fan is hardly noticeable at the low speed and only draws about 40W. I like the idea of continuing to circulate air but the humidifier wants to run all the time which wastes water and it's probably not efficient at humidifying when the fan is running slow and the furnace burner is off.

(It would make sense to me if the furnace had an option to power the humidifier only when the fan was at high speed. I checked the installation manual but didn't see such an option.)

My solution is to sense the temperature in the plenum to determine when the burner is running (and the fan is therefore running at high speed). I already had a [Maxim Integrated DS18B20 sensor](https://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/DS18B20.html) installed in the plenum, connected to another controller with an XBee. Therefore the humidifier controller also has an XBee and I modified the other unit to send the temperature to it every 10 seconds.

This is probably an interim solution; I would prefer a single unit with the temperature sensor directly connected but this was the quickest implementation for a couple of reasons. The final design will also have an XBee so that data can be logged to an online service (I use [GroveStreams](https://grovestreams.com/)).

### [State Transition Diagram](https://raw.githubusercontent.com/JChristensen/humid_ctrlr/master/state_transition_diagram.png) - Notes###

- The state machine determines whether the temperature is increasing or decreasing simply by comparing the temperature on the last trip through the machine to the current temperature.
- The "Stale Data" state ensures the humidifier is shut off if data is not received from the sensor node. All other states include a check for stale data. In the initial implementation, data is sent every 10 seconds, and a stale data condition occurs if more than 30 seconds elapse without receipt of data.
- T-HI represents the plenum temperature at which the blower turns off at the end of a heating cycle. Through observation, this occurs between about 95°F and 100°F.
- T-LO represents a temperature that the plenum will usually fall below between heating cycles. When the temperature increases above this level, we conclude that the burner and fan are on.
- Note that in situations where the furnace is making up a larger temperature difference, the plenum temperature may not fall below T-LO. The transition between the 5.OFF (temp. decreasing) state and the 3.ON (temp. increasing) state is to respond to this condition.
- The initial implementation uses T-LO = 80°F and T-HI = 95°F.

### Revision History ###

**Jan 2016** -- Initial implementation.
### Acknowledgements ###
Many thanks to Tom (a.k.a. Ducky) for providing hardware that made for a rapid and robust implementation, including: MCU/XBee board, Relay board, LCD I2C backpack.

### CC BY-SA ###
Humidifier Controller by Jack Christensen is licensed under CC BY-SA 4.0, http://creativecommons.org/licenses/by-sa/4.0/
