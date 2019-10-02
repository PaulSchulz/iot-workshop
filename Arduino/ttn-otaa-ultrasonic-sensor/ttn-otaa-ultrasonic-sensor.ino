// https://www.tutorialspoint.com/arduino/arduino_ultrasonic_sensor.htm

/*******************************************************************************
   Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
   Copyright (c) 2018 Terry Moore, MCCI
   Copyright (c) 2018 Betina Wendel and Thomas Laurenson

   Permission is hereby granted, free of charge, to anyone
   obtaining a copy of this document and accompanying files,
   to do whatever they want with them without any restriction,
   including, but not limited to, copying, modification and redistribution.
   NO WARRANTY OF ANY KIND IS PROVIDED.

   Sketch Summary:
   Target device: Dragino LoRa Shield (US900) with Arduino Uno
   Target frequency: AU915 sub-band 2 (916.8 to 918.2 uplink)
   Authentication mode: Over the Air Authentication (OTAA)

   This example requires the following modification before upload:
   1) Enter a valid Application EUI (APPEUI)
      For example: { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
   2) Enter a valid Device EUI (DEVEUI)
      For example: { 0x33, 0x22, 0x11, 0x11, 0x88, 0x88, 0x11, 0x22 };
      This is little endian format, so it is in reverse order that the server
      provides. In the example above, the original value was: 2211888811112233
   3) Enter a valid Application Key (APPKEY)
      For example: { 0xe4, 0x17, 0xd3, 0x3b, 0xef, 0xf3, 0x80, 0x7c, 0x7c, 0x6e, 0x42, 0x43, 0x56, 0x7c, 0x21, 0xa7 };

   The DEVEUI and APPKEY values should be obtained from your LoRaWAN server
    (e.g., TTN or any private LoRa provider). APPEUI is set to zeroes as
   the LoRa Server project does not requre this value.

 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include <EEPROM.h> // To get configuration details from EEPROM

// Application Pins
int pingPin  = 3;   // Pin for sensor
int echoPin  = 4;
long duration = 0;
long inches, cm;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LoRaWAN
// Default parameters
char magic[7] = "paeiot";
char version  = 0;         // EEPROM Map Version
String devname    = "iot-workshop-1";
static byte deveui[8]  = { 0x01, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 };
static byte appeui[8]  = { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
static byte appkey[16] = { 0xF9, 0xF0, 0xEA, 0x29, 0xD5, 0x38, 0x73, 0x56, 0xFB, 0x8B, 0x28, 0xB8, 0x16, 0x60, 0x2F, 0x8D };
int    magic_check     = 1; // Set to 1 (true) if EEPROM magic is set.

//
// For normal use, we require that you edit the sketch to replace FILLMEIN
// with values assigned by the TTN console. However, for regression tests,
// we want to be able to compile these scripts. The regression tests define
// COMPILE_REGRESSION_TEST, and in that case we define FILLMEIN to a non-
// working but innocuous value.
//
//#ifdef COMPILE_REGRESSION_TEST
//# define FILLMEIN 0
//#else
//# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
//# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
//#endif

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) {
  if (magic_check == 0) {
    memcpy_P(buf, APPEUI, 8);
  } else {
    memcpy(buf, appeui, 8);
  }
}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x01, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 };
void os_getDevEui (u1_t* buf) {
  if (magic_check == 0) {
    memcpy_P(buf, DEVEUI, 8);
  } else {
    memcpy(buf, deveui, 8);
  }
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from the TTN console can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0xF9, 0xF0, 0xEA, 0x29, 0xD5, 0x38, 0x73, 0x56, 0xFB, 0x8B, 0x28, 0xB8, 0x16, 0x60, 0x2F, 0x8D };
void os_getDevKey (u1_t * buf) {
  if (magic_check == 0) {
    memcpy_P(buf, APPKEY, 16);
  } else {
    memcpy(buf, appkey, 16);
  }
}

void read_eeprom (void) {
  int addr;
  int size;
  int ssize;
  char tmp[16];

  Serial.println(F("Reading EEPROM"));

  // magic
  Serial.print("  magic:      ");
  addr = 0; size = 7;
  for (int i = 0; i < size; i++) {
    magic[i] = EEPROM.read(addr + i);
  }
  Serial.print(magic);
  Serial.println();

  // version
  Serial.print("  version:    ");
  addr = 7;
  version = EEPROM.read(addr);
  Serial.print(version, DEC);
  Serial.println();

  // devname
  Serial.print("  devname:    ");
  addr = 8; size = 24;
  for (int i = 0; i < size; i++) {
    devname[i] = EEPROM.read(addr + i);
  }
  Serial.print(devname);
  Serial.println();

  // deveui
  Serial.print("  deveui:     ");
  addr = 32; size = 8;
  for ( int i = 0; i < size; i++ ) {
    deveui[i] = EEPROM.read(addr + i);
    sprintf(tmp, "0x%.2X", deveui[i]);
    Serial.print(tmp);
    Serial.print(" ");
  }
  Serial.println();

  // appeui
  Serial.print("  appeui:     ");
  addr = 40; size = 8;
  for ( int i = 0; i < size; i++) {
    appeui[i] = EEPROM.read(addr + i);
    sprintf(tmp, "0x%.2X", appeui[i]);
    Serial.print(tmp);
    Serial.print(" ");
  }
  Serial.println();

  // appkey
  Serial.print("  appkey:     ");
  addr = 48; size = 16;
  for ( int i = 0; i < size; i++) {
    appkey[i] = EEPROM.read(addr + i);
    sprintf(tmp, "0x%.2X", appkey[i]);
    Serial.print(tmp);
    Serial.print(" ");
  }
  Serial.println();

  if (magic[0] == 'p'
      && magic[1] == 'a'
      && magic[2] == 'e'
      && magic[3] == 'i'
      && magic[4] == 'o'
      && magic[5] == 't') {
    Serial.println("EEPROM Magic matches");
    Serial.println("  Using stored EEPROM values");
    magic_check = 1;
  } else {
    Serial.println("EEPROM Magic does NOT matches");
    Serial.println("  Using default values");
  }

  Serial.println();
}

static uint8_t mydata[16] = "LoRaWAN Sensor";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping for Dragino Lorashield
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 9,
  .dio = {2, 6, 7},
};

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      {
        u4_t netid = 0;
        devaddr_t devaddr = 0;
        u1_t nwkKey[16];
        u1_t artKey[16];
        LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
        Serial.print("netid: ");
        Serial.println(netid, DEC);
        Serial.print("devaddr: ");
        Serial.println(devaddr, HEX);
        Serial.print("artKey: ");
        for (int i = 0; i < sizeof(artKey); ++i) {
          Serial.print(artKey[i], HEX);
        }
        Serial.println("");
        Serial.print("nwkKey: ");
        for (int i = 0; i < sizeof(nwkKey); ++i) {
          Serial.print(nwkKey[i], HEX);
        }
        Serial.println("");
      }
      Serial.println(F("Successful OTAA Join..."));
      // Disable link check validation (automatically enabled
      // during join, but because slow data rates change max TX
      // size, we don't use it in this example.
      LMIC_setLinkCheckMode(0);
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_RFU1:
      ||     Serial.println(F("EV_RFU1"));
      ||     break;
    */
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.print(F("Received "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    /*
      || This event is defined but not used in the code. No
      || point in wasting codespace on it.
      ||
      || case EV_SCAN_FOUND:
      ||    Serial.println(F("EV_SCAN_FOUND"));
      ||    break;
    */
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned) ev);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void do_send(osjob_t* j) {


  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {

   sprintf(mydata, "Dist: %d cm", cm);

//    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    LMIC_setTxData2(1, mydata, strlen(mydata), 0);
    Serial.println(F("Packet queued"));
    Serial.print(F("Sending packet on frequency: "));
    Serial.println(LMIC.freq);
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));

  read_eeprom();

#ifdef VCC_ENABLE
  // For Pinoccio Scout boards
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
#endif

#if defined(CFG_au921)
  Serial.println(F("Loading AU915/AU921 Configuration..."));
#endif

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  LMIC_setDrTxpow(DR_SF7, 14);
  LMIC_selectSubBand(1);
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();

  //Serial.println(analogRead(A0));
  //Serial.println(analogRead(A1));
  //Serial.println(analogRead(A2));
  //Serial.println(analogRead(A3));
  //Serial.println(analogRead(A4));
  //Serial.println(analogRead(A5));

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  inches = duration / 74 / 2;
  cm = duration / 29 / 2;

  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

}
