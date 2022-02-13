# Firmware Requirements

This project needs to be low-power, low-data, and somewhat resilient. As such, this document examines
the firmware requirements needed to achieve this.

There are two modes of operation: low-accuracy, and high-accuracy.

__Low accurracy__ mode is when we've detected no movement. We're just letting the server know we're still here, and our battery level.
We may want a number of tries to get a GPS fix, and if we fail, just get gsmLoc instead. Then, send the battery level and go back to sleep.

__High accuracy__ mode is when we're moving. A second timer interrupt is configured to upload movement every 30s(~?). Keep all radios on!

## Setup
At setup, we do the following:

 - Initialise pins: set pinModes for I/O on FONA and Accelerometer
 - Hardware check. Are the peripherals present?
 - Hardware initialisation: run initialisation routines
 - Initialise interrupts: attachInterrupt for timers and accelerometer

## Main loop

Most of the time, we want to deep-sleep here.

The sleep will be interrupted and the loop will run when the following happens:

 - A timer interrupt happened
 - Movement happened

## States
Build a [transition table](https://www.adamtornhill.com/Patterns%20in%20C%202,%20STATE.pdf) for our state machine.

States to cover:

 - Trying to get first GPS fix
 - Got GPS first fix (have GPSdata)
 - Turn on GPRS radio (don't advance until active)
 - Transmit current position over HTTP
 - Shut down GPRS radio

We will want to keep the GPRS radio on while movement is happening.


## Interrupts

### Timers

It's time to wake up and check we have a fix. If we do, then upload that.

### Accelerometer movement

Sets the current time for last movement, and toggle the interrupt flag


## Questions

 - Does getGSMLoc() set AGPS data? Can we get a faster fix using it?


## Nice-to-haves

SMS command and control:
  - Ask to go into high-accuracy for a bit.
  - Set off a siren or buzzer (e.g. if I'm outside the thief's flat...)

