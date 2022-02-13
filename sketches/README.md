# Arduino sketches
Proof-of-concept and implementation Arduino sketches for the FONA and
accelerometer hardware.

## accel-interrupt
Uses a modified Sparkfun Library to read the MMA8452Q via the interrupt pin.
Install this library from [sjmf/SparkFun_MMA8452Q_Arduino_Library](https://github.com/sjmf/SparkFun_MMA8452Q_Arduino_Library)
by downloading it as a zip file, then click "Sketch > Include Library > Add .ZIP Library..."
within Teensyduino. Note that this lib acts as a replacement and will conflict with the original
Sparkfun Lib.

## track-gps
A basic implementation of the GPS tracking functionality using a state machine

