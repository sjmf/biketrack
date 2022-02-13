/**
 * GPS tracker using Adafruit FONA 808 cellular module
 * 
 * Uses boilerplate code from the Adafruit FONAtest.c example
 * 
 * @author Samantha Finnigan
 * @date 2021
 */
#include <SparkFun_MMA8452Q.h>

#include <avr/sleep.h>
#include "Adafruit_FONA.h"

// Configure input pins depending on platform:
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
#include <SoftwareSerial.h>

  // ESP8266 pins
  #define FONA_RST 14
  #define FONA_RX  12
  #define FONA_TX  13
  
  SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
  SoftwareSerial *fonaSerial = &fonaSS;

#else
  // On Leonardo/M0/etc, others with hardware serial, use hardware serial!
  HardwareSerial *fonaSerial = &Serial1;
  
  // Teensy 3.1 hardware serial pins
  #define FONA_RX  0
  #define FONA_TX  1
  #define FONA_RST 2
#endif

#define FONA_RI  3
#define FONA_KEY 4
#define FONA_PS  5
#define TEENSY_LED 13

#define GPRS_APN  "giffgaff.com"
#define GPRS_USER "gg"
#define GPRS_PASS "p"

#define POST_URL "http://example.com/track/"
#define TRACK_MS 300000 // 5 minutes
#define IDLE_MS  10000
#define RETRIES  10

// Uncomment to disable LEDs, pins, etc
//#define LOW_POWER

char debug[128];

// Define for FONA 808/800
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t type = FONA808_V2;

// State machine
typedef enum 
{
  GPS_INIT,
  GPRS_INIT,
  GPS_WAIT,
  HTTP_POST
} State;

State state = GPS_INIT;


/****************************************************************
 * Do program setup work: enable serial output, detect FONA board etc.
 * Runs once.
 */
void setup() {
  
  Serial.begin(115200);
  
  pinMode(TEENSY_LED, OUTPUT);
  pinMode(FONA_KEY, OUTPUT);
  pinMode(FONA_PS, INPUT);
  pinMode(FONA_RI, INPUT);
  
  turnFONAon();
  
  // Print module IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }
  
  fona.setGPRSNetworkSettings(F(GPRS_APN), F(GPRS_USER), F(GPRS_PASS));
  
  // HTTPS not (currently?) working? https://forums.adafruit.com/viewtopic.php?f=19&t=59358
//  fona.setHTTPSRedirect(true); 
}


/****************************************************************
 * Main loop
 */
void loop() {
  char gpsdata[120] = "";
  char *buffer;
  char *url;
  int16_t stat = 0;
  int8_t retries = 0;
  
  while (1) {
    
    switch(state) {
      // Set up 
      case GPS_INIT:
        startGPS();
        state = GPRS_INIT;
        //break; // No need. Fall through.

      case GPRS_INIT:
        if(startGPRS()) { // Got home network association and GPRS OK!
          state = GPS_WAIT;
        } else {
          break; // Fall through if state progressed!
        }
        
      case GPS_WAIT:
        buffer = gpsdata;
        do {
          stat = getBatteryVoltage(buffer);
          delay(1000); // Wait for FONA to wake up
        } while (!stat);
        
        buffer = &buffer[stat];
        Serial.println(stat);
        
        strcpy(buffer, "gps=");       // 4 characters = 4 bytes,
        buffer = &buffer[4];          //    so advance buffer by 4...
        stat = getGPSLoc(buffer);     // Write after POST req variable
        
        // Wait for a fix...
        // If our GPS data starts (after position 4 (+ buffer offset)) '1980', 
        // then we have a cold boot and need to wait longer for a fix
        if(stat >=2) {
          state = HTTP_POST;
        } else {
          break;  // Break in else: otherwise, fall through.
        }

      case HTTP_POST:
        url = (char*)POST_URL;
        
        if(!httpPOST(gpsdata, url)) {
          // Weird unrecoverable network error stuff? Try again or reboot.
          if(retries > RETRIES) {
            softReset();
          }
          retries++;
          sprintf(debug, "Retries remaining: %d/%d; status: %d", RETRIES - retries, RETRIES, stat);
          Serial.println(debug);
          break;
        }
        
        state = GPS_WAIT;
        break;

//      case SHUTDOWN:
//        disableGPRS();
//        turnFONAoff();
//        break;
      
      default: break;
    }


    // Sleep length for GPS_WAIT is longer:
    if (state == GPS_WAIT) {
//      turnFONAoff();
      delay(TRACK_MS);
//      turnFONAon();
    } else {
      delay(IDLE_MS);
    }
    
    flashLED();
    idle();
  }
}


/****************************************************************
 * Start up Global Positioning Satellite receiver
 */
void startGPS() {
  // Turn GPS on
  Serial.println(F("Enabling GPS..."));
  if (!fona.enableGPS(true))
    Serial.println(F("Failed to enable GPS"));
  else
    Serial.println(F("GPS Enabled"));
}


/****************************************************************
 * Start up GPRS
 */
bool startGPRS() {
  uint8_t stat = checkNetStatus();

  // First, check if we're associated with the network:
  if(stat == 1 || stat == 5) {
    // Got network association OK, now try to enable GPRS:
    if (fona.enableGPRS(true)) {
      Serial.println(F("GPRS radio enabled"));
      return true;
    }
    
    Serial.println(F("Failed to enable GPRS"));
  }
  return false;
}


/****************************************************************
 * Turn off GPRS
 */
void disableGPRS() {
  // turn GPRS off
  if (!fona.enableGPRS(false))
    Serial.println(F("Failed to turn off"));
}


/****************************************************************
 * Check the data network association status
 */
int8_t checkNetStatus() {
  uint8_t stat = fona.getNetworkStatus();
  Serial.print(F("Network status "));
  Serial.print(stat);
  Serial.print(F(": "));
  
  if (stat == 0) Serial.println(F("Not registered"));
  if (stat == 1) Serial.println(F("Registered (home)"));
  if (stat == 2) Serial.println(F("Not registered (searching)"));
  if (stat == 3) Serial.println(F("Denied"));
  if (stat == 4) Serial.println(F("Unknown"));
  if (stat == 5) Serial.println(F("Registered roaming"));
  
  return stat;
}


/****************************************************************
 * Read the battery voltage and percentage
 * Writes 17 characters into the provided buffer
 * Returns the number of characters written
 */
uint8_t getBatteryVoltage(char *buffer){
  uint16_t vbat;
  if (fona.getBattVoltage(&vbat)) {
    sprintf(buffer, "vbat=%04u&", vbat); // 10 characters fixed width
  } else {
    Serial.println(F("Failed to read Batt"));
    return 0;
  }

  if (fona.getBattPercent(&vbat)) {
    sprintf(&buffer[10], "vpc=%02u&", vbat); // 7 chars fixed width
  } else {
    Serial.println(F("Failed to read Batt"));
  }

  Serial.println(buffer);
  return strlen(buffer);
}



/****************************************************************
 * Get a GPS location
 */
int8_t getGPSLoc(char gpsdata[]) {
  int8_t stat = gpsStatus();
  
  // If we have a fix, get our GPS location and store it in the passed char[] array!
  if(stat >= 2) {
    // Also log it to serial ;)
    fona.getGPS(0, gpsdata, 120);
    Serial.println(F("Reply in format: mode,fixstatus,utctime(yyyymmddHHMMSS),latitude,longitude,altitude,speed,course,fixmode,reserved1,HDOP,PDOP,VDOP,reserved2,view_satellites,used_satellites,reserved3,C/N0max,HPA,VPA"));
    Serial.println(gpsdata);
  }
  return stat;
}


/****************************************************************
 * GPS Status
 */
int8_t gpsStatus() {
  // check GPS fix
  int8_t stat;
  stat = fona.GPSstatus();
  if (stat < 0)
    Serial.println(F("Failed to query"));
  if (stat == 0) Serial.println(F("GPS off"));
  if (stat == 1) Serial.println(F("No fix"));
  if (stat == 2) Serial.println(F("2D fix"));
  if (stat == 3) Serial.println(F("3D fix"));
  
  return stat;
}


/****************************************************************
 * Do HTTP Post
 */
uint16_t httpPOST(char data[], char url[]) {
  // Post data to website
  uint16_t statuscode = 0;
  int16_t length;

  flushSerial();
  Serial.println(data);
  
  Serial.println(F("****"));
  
  
  if (!fona.HTTP_POST_start(url, F("application/x-www-form-urlencoded"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length)) {
    Serial.print(F("Failed post to "));
    
    sprintf(debug, "Status: %d", statuscode);
    Serial.println(debug);
    return 0;
  }
  
  sprintf(debug, "Response body length: %d", length);
  Serial.println(debug);
  
  while (length > 0) {
    while (fona.available()) {
      char c = fona.read();

// Slow serial. Write directly to serial register on supported platforms
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
      loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
      UDR0 = c;
#else
      Serial.write(c);
#endif

      length--;
      if (! length) break;
    }
  }
  Serial.println(F("\n****"));
  fona.HTTP_POST_end();
  
  return statuscode;
}


/****************************************************************
 * Hush, now...
 * https://www.pjrc.com/teensy/low_power.html
 */
void idle() {
  set_sleep_mode(SLEEP_MODE_IDLE);
  noInterrupts();
  sleep_enable();
  interrupts();
  sleep_cpu();
  sleep_disable();
}


/****************************************************************
 * LED routines:
 */
void flashLED() {
#ifndef LOW_POWER
  digitalWrite(TEENSY_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(TEENSY_LED, LOW);    // turn the LED off by making the voltage LOW
#endif
}
void ledON() {
#ifndef LOW_POWER
  digitalWrite(TEENSY_LED, HIGH);
#endif
}
void ledOFF() {
#ifndef LOW_POWER
  digitalWrite(TEENSY_LED, LOW);
#endif
}


/****************************************************************
 * Pulse KEY pin for 2 seconds to power up/down module
 * Provides convenience methods which check PS pin first
 */
void _toggleFONA() {
  Serial.println("Pulling KEY pin...");
  delay(1000);
  digitalWrite(FONA_KEY, HIGH);
  delay(1000);
  digitalWrite(FONA_KEY, LOW);
  delay(2000);
  digitalWrite(FONA_KEY, HIGH);
  delay(1000);
}
uint8_t _getFONAPowerStatus() {
  return digitalRead(FONA_PS);
}
void turnFONAon() {
  if (! _getFONAPowerStatus()) {
    // Fona is off if FONA_PS is low
    Serial.println("Bringing FONA up...");
    ledON();
    _toggleFONA();
    
    ledOFF();
  } else {
    // FONA is already on!
    Serial.println("FONA is already on!");
  }

  // Bootstrap FONA
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    ledOFF();
    delay(100);
    softReset();
  }
}
void turnFONAoff() {
  if (_getFONAPowerStatus()) {
    // Fona is on if FONA_PS is high
    Serial.println("Putting FONA to sleep...");
    _toggleFONA();
  } else {
    // FONA is already off!
    Serial.println("FONA is already off!");
  }
}

/****************************************************************
 * Flush Serial
 */
void flushSerial() {
  while (Serial.available())
    Serial.read();
}

/****************************************************************
 * Soft reset Teensy
 */
#define SCB_AIRCR (*(volatile uint32_t *)0xE000ED0C) // Application Interrupt and Reset Control location
void softReset() {
  Serial.end();  //clears the serial monitor  if used
  SCB_AIRCR = 0x05FA0004;
}
