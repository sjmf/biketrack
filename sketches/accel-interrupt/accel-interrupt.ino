
// https://www.pjrc.com/teensy/interrupts.html
#include <avr/io.h>
#include <avr/interrupt.h>

#include <Wire.h>                 // Include Wire library for I2C
#include "SparkFun_MMA8452Q.h"    // Uses forked library for interrupt support. Install: https://github.com/sjmf/SparkFun_MMA8452Q_Arduino_Library
#include <avr/sleep.h>

#define MMA_PIN_INT1 20
#define MMA_PIN_INT2 21
#define TEENSY_LED 13

#define MOTION_THRESHOLD 0x10
#define MOTION_DEBOUNCE  0x30

MMA8452Q accel;

volatile uint8_t interrupt = 0;
uint8_t source = 0;

void setup() {
  pinMode(MMA_PIN_INT1, INPUT_PULLUP);
  pinMode(MMA_PIN_INT2, INPUT_PULLUP);
  pinMode(TEENSY_LED, OUTPUT);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  
  // NB: maybe use accel.init() instead for fine-control of SCALE and DATA RATE. See Example7_OriginalDeprecated in SparkFun MMA lib
  if (accel.init(SCALE_4G, ODR_800) == false) {
    Serial.println("Not Connected. Please check connections and read the hookup guide.");
    while (1);
  }
  
  Serial.println("Setting up interrupts!");
  accel.setInterruptsEnabled(INT_FREEFALL_MOTION);
  accel.configureInterrupts(false, false); // Active HIGH (pulled high = interrupt). Push-pull configuration.
  accel.setInterruptPins(false, false, false, false, false, false); // True = pin 1 (here freefall_motion connected to MMA_PIN_INT1)
  accel.setupMotionDetection(MOTION_THRESHOLD, MOTION_DEBOUNCE); // motionThreshold, debounceCount

  // https://forum.pjrc.com/threads/42800-Teensy-Interrupts-for-Dummies
  attachInterrupt(digitalPinToInterrupt(MMA_PIN_INT1), isr, FALLING);
  
  Serial.println("Setup done");
}

void idle() {
  set_sleep_mode(SLEEP_MODE_IDLE);
  noInterrupts();
  sleep_enable();
  interrupts();
  sleep_cpu();
  sleep_disable();
}

void loop() {
  
  // put your main code here, to run repeatedly:
  if(interrupt) {
    Serial.print("Service Interrupt: ");
    if (accel.available()) {      // Wait for new data from accelerometer
      // Acceleration of x, y, and z directions in g units
      Serial.print(accel.getCalculatedX(), 3);
      Serial.print("\t");
      Serial.print(accel.getCalculatedY(), 3);
      Serial.print("\t");
      Serial.print(accel.getCalculatedZ(), 3);
      Serial.println();
    }

    printMotionSource();

    
    noInterrupts(); // cli();
    interrupt = 0;
    interrupts();   // sei();
  }

  idle();
}

void isr() {
  interrupt++;
  digitalWrite(TEENSY_LED, !digitalRead(TEENSY_LED));
}

void printMotionSource() {
  source = accel.readMotionSourceRegister();
    
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

  Serial.printf("%d. calls. FF_MT_SRC: 0x%02x -- 0b" BYTE_TO_BINARY_PATTERN "\n", 
    interrupt, source, BYTE_TO_BINARY(source)
  ); 
}
