# BikeTrack
A homebrew GPS Tracker using Teensy 3.x and SIM808.

This project aims to implement a basic GPS tracker with the above hardware. There
is a requirement that this runs from a battery, potentially using the bike dynamo 
to charge while riding.

In 2020 my bike was stolen from the roof of my car. Although I later found where it
was located, due to police incompetence it still has not been recovered. I bought
a new bike, and set about building an asset tracker with electronics hobbyist parts.

This repository contains WIP code and associated files for a basic asset tracking
system using GPS. As a work-in-progress, it is something I am tinkering with in
my spare time. While I'm good at finishing projects, I also tend to underestimate
the timescales and labour required to get there, so recognise the idiom that the 
last 10% takes 90% of the time...

## Hardware Selection
I received the [Adafruit SIM808](https://learn.adafruit.com/adafruit-fona-808-cellular-plus-gps-breakout/overview)
as a thoughtful christmas gift from my partner. I had a Teensy 3.1 in my electronics
storage, hence the selection of this microcontroller. The Sparkfun [MMA8452Q Accelerometer](https://www.sparkfun.com/products/12756)
was the only part I've had to actually buy so far!

The current status of this hardware is breadboarded up. Firmware is under implemetation.
The plan is to design and fabricate a simple PCB to solder these components directly on to, 
and 3d-print a housing/case using my [Prusa Mini+](https://www.prusa3d.com/product/original-prusa-mini-8/).

## Project Roadmap

- [x] Hardware Selection
  - [x] [Teensy 3.x](https://www.pjrc.com/store/teensy32.html)
  - [x] [Adafruit SIM808](https://learn.adafruit.com/adafruit-fona-808-cellular-plus-gps-breakout/overview)
  - [x] [MMA8452Q Accelerometer](https://www.sparkfun.com/products/12756)
- [ ] Firmware Implementation (breadboard)
  - [x] SIM808 [Proof of Concept](https://forums.adafruit.com/viewtopic.php?f=54&t=187767&p=910403)
    - [x] Power on programatically using KEY pin
    - [x] Cellular + GPS functionality
    - [x] Run from battery
  - [x] MMA8452Q Proof of Concept
    - [x] Interrupt-driven code PoC ([modified SparkFun Library](https://github.com/sjmf/SparkFun_MMA8452Q_Arduino_Library))
  - [ ] Low Power use on Teensy using [duff2013/Snooze](https://github.com/duff2013/Snooze)
  - [ ] Code Integration (see [Firmware Requirements](Requirements.md))
- [ ] Hardware implementation
  - [x] Breadboard PoC
  - [ ] Schematic design
  - [ ] PCB Design / Layout
  - [ ] Order PCBs
  - [ ] Solder up
- [x] Serverside functionality
  - [x] SIM808 POST to server
  - [x] Store GPS points in [MongoDB Atlas](https://www.mongodb.com/cloud/atlas/)
  - [x] Retrieve GPX Track
  - [x] Plot GPX track in OpenLayers
  - [ ] Security (currently no authentication, no client SSL, replay attacks possible)
  - [ ] Add field 'segmentstart' to new track segments when storing
- [ ] Housing design
  - [ ] CAD housing for project
  - [ ] 3D-print case

