# mavleash

Author: Todd Stellanova <todd@droneflow.com>
License: MIT

### Description 

This Particle application allows you to monitor the status of your drone,
including position, velocity, and battery level, anywhere there is cellular network coverage.
mavleash stands for "Micro Air Vehicle Leash"

Mavleash uses the [Particle.io Electron board ](https://store.particle.io/collections/electron), 
with the associated cellular data plan, to maintain two-way radio contact with your drone,
even if you lose your RC connection.  Unlike traditional telemetry radios such as SiK, 
the range of the Electron is only limited by the availability of mobile phone towers. 

### Compatibility

This project uses the [mavlink protocol](https://github.com/mavlink/mavlink) v1.0
to communicate with any drone that supports it (there are many).
For my testing I've used standard Pixhawk v2.4.6 autopilot hardware and the latest
"Stable" PX4 firmware build installed using [QGroundControl](http://qgroundcontrol.org/downloads).

### Building and Flashing

Currently you will need to build using the Particle command line tools:

```
cd /firmware
rm *.bin
particle compile electron .
particle flash <electron device ID> electron_firmware_*.bin
```

Assuming your Electron is connected to the cellular network (the status LED is "breathing cyan"),
you can flash your Electron over the cellular connection (data usage charges apply).

### Wiring

This application connects from the drone's mavlink serial output port to the 
serial UART port of the Electron (RX, TX, GND, VIN).  
Assuming you've already configured your drone to output mavlink on the serial port (not USB),
you need only run four wires from the drone's port to the Electron pins. 
In my testing I found it perfectly adequate to attach the Electron's VIN to the Pixhawk's 5V output
line on the TELEM1 port. 

Tip: If you think you've wired up everything correctly and you're not receiving any data from the
Electron, you may have swapped the RX and TX lines: try reversing them. 

### Running

The included `/web/mavleash.html` file provides an example of monitoring the status of your drone 
using a simple web page with javascript.  
To use it, you'll need to obtain your Electron's device ID and authorization token from 
the Particle.io site and populate those fields in the HTML file.
This file is for test purposes only: be careful to never publish your authorization token.
Follow the directions from Particle.io if you wish to create a full-blown mobile app or website
that monitors your device's location.

In addition, you can use IFTTT.com (if this then that) to pull Particle.io events into other web services.
I've setup an [example IFTTT recipe](https://ifttt.com/recipes/397072-log-particle-event-data) 
that will pull particle event data (I use the 'statecsv' event mavleash generates) into 
a Google Drive spreadsheet: each time mavleash generates a statecsv event, this recipe will
add a row to the spreadsheet.  This is handy for recording your flights.

You can setup a similar IFTTT recipe for tracking your flight time:
mavleash also publishes the 'LANDED_STATE' event when your drone takes off (IN\_AIR) or lands (ON\_GROUND).


### Next Steps

Some folks may wish to run both mavleash as well as a more traditional telemetry radio such as the SiK 915MHz/433MHz,
or monitor their mavlink telemetry using MinimOSD. 

You can readily split the output line (RX) of the autopilot mavlink port and 
supply it to multiple readers (SiK, MinimOSD, and Electron).  
This will provide you with a telemetry stream from the autopilot.  
You can configure the wiring so that one of the radios can write commands back to the autopilot's input pin.
For example you could use the [MAVBoard Lite](https://github.com/mavboard/mavboard)

However, if you wish to send commands to the autopilot from multiple sources, this is more tricky.
One option would be to use a digital multiplexer (eg NC7SZ157P6X) to switch between, say, the SiK 
and the Electron.  You could output a mux control signal from one of the Electron's digital output
pins so that when the Electron is sending commands, it owns the autopilot's input pin. 

