#include <EEPROM.h>
#include "devices.h"

// Write persistant data to Arduino EEPROM for PAE IoT applications,
// connecting to 'The Things Network' (TTN).
//
// This is useful in that the firmware program can be reflashed,
// but the LoRaWAN node configuration/authentication data is maintained
// on each device.
//
// The source code then does not need to be different for each device.

// If device number is 0, use default settings, otherwise set configuration from device array.
int    device_number = 0;  // Device number for configuration data

// Software Configuration
int eeprom_write = 0; // Write EEPROM, otherwise, just read.
int eeprom_erase = 0; // Erase all EEPROM contents before writing
int debug = 0;        // Enable/Disable debug output

int addr = 0;         // Address to read/write EEPROM data
int size = 0;

// EEPROM structure
// ----------------
// magic[7]      0-6   "paeiot", null terminated string
// version         7   \0 - integer, which is this EEPROM format version (initially 0)
// devname[24]  8-31   string, null terminated
// deveui[8]   32-39   Device Extended Unique Identifier, from TTN in little endian format.
// appeui[8]   40-47   Application Extended Unique Identifier, from TTN in little endian format.
// appkey[16]  48-63   Application Key, used for OTAA, from TTN in big endian format.

// Default parameters
char magic[7] = "paeiot";
int  version  = 0;

// Device parameters
String devname = "                ";
byte   deveui[8];
byte   appeui[8];
byte   appkey[16];
 
// Default Device Parameters

// EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.



iot_otaa_dev device = {
  "paeiot",
  0,
  "iot-mawsonlakes-1",
  { 0x0E, 0xE9, 0x2E, 0x60, 0x21, 0x1A, 0x66, 0x00 },
  { 0x12, 0xA0, 0x02, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
  { 0x3F, 0xE1, 0xCD, 0x5C, 0xDF, 0x39, 0xE7, 0x6B, 0xBF, 0xF1, 0xF4, 0x01, 0x70, 0xF7, 0x11, 0x30 }
};

void display_device_configuration ( iot_otaa_dev* dev) {
  char tmp[16];
  Serial.print("  Device Name:     ");
  Serial.println(dev->devname);

  Serial.print("  Device EUI:      ");
  size = 8;
  for ( int i = 0; i < size; i++) {
    sprintf(tmp, "0x%.2X", (dev->deveui)[i]);
    Serial.print(tmp);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("  Application EUI: ");
  size = 8;
  for ( int i = 0; i < size; i++) {
    sprintf(tmp, "0x%.2X", (dev->appeui)[i]);
    Serial.print(tmp);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("  Application Key: ");
  size = 16;
  for ( int i = 0; i < size; i++) {
    sprintf(tmp, "0x%.2X", (dev->appkey)[i]);
    Serial.print(tmp);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println();
};

// Read EEPROM ///////////////////////////////////////////
void read_eeprom (){
  unsigned int tmp[16];
  char tmp_str[40];

  Serial.println(F("Reading EEPROM"));

  // magic
  Serial.print("  magic:      ");
  addr = 0; size = 7;
  for (int i = 0; i < size; i++) {
    tmp_str[i] = EEPROM.read(addr + i);
  }
  Serial.print(tmp_str);
  Serial.println();
  
  // version
  Serial.print("  version:    ");
  addr = 7;
  tmp[0] = EEPROM.read(addr);
  Serial.print(tmp[0], DEC);
  Serial.println();

  // devname
  Serial.print("  devname:    ");
  addr = 8; size = 24;
  for (int i = 0; i < size ; i++) {
    tmp_str[i] = EEPROM.read(addr + i);
  }
  tmp_str[size-1] = 0; // Force string termination
  Serial.print(tmp_str);
  Serial.println();
  
  // deveui
  Serial.print("  deveui:     ");
  addr = 32; size = 8;
  for ( int i = 0; i < size; i++) {
    tmp[i] = EEPROM.read(addr + i);
    sprintf(tmp_str, "0x%.2X", tmp[i]);
    Serial.print(tmp_str);
    Serial.print(" ");
  }
  Serial.println();
 
  // appeui
  Serial.print("  appeui:     ");
  addr = 40; size = 8;
  for ( int i = 0; i < size; i++) {
    tmp[i] = EEPROM.read(addr + i);
    sprintf(tmp_str, "0x%.2X", tmp[i]);
    Serial.print(tmp_str);
    Serial.print(" ");
  }
  Serial.println();

  // appkey
  Serial.print("  appkey:     ");
  addr = 48; size = 16;
  for ( int i = 0; i < size; i++) {
    tmp[i] = EEPROM.read(addr + i);
    sprintf(tmp_str, "0x%.2X", tmp[i]);
    Serial.print(tmp_str);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println();
  
};

// Write EEPROM ///////////////////////////////////////////
int write_eeprom (iot_otaa_dev device, int eeprom_erase){
  String devname = "                ";
  byte   deveui[8];
  byte   appeui[8];
  byte   appkey[16];
   
  devname = device.devname;
  memcpy(deveui,  device.deveui,  8);
  memcpy(appeui,  device.appeui,  8);
  memcpy(appkey,  device.appkey,  16);
  
  if (eeprom_erase) {
    Serial.println("  *** Erasing EEPROM");
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    };
  };

  // Write magic
  addr = 0 ; size = 7;
  for (int i = 0 ; i < size ; i ++) {
    EEPROM.write(addr + i, magic[i]);
  };

  // Write version
  addr = 7;
  EEPROM.write(addr, version);

  // Write devname
  addr = 8; size = 24;
  for (int i = 0 ; i < size ; i ++) {
    EEPROM.write(addr + i, devname[i]);
  };
  
  // Write deveui
  addr = 32; size = 8;
  for (int i = 0 ; i < size ; i ++) {
    EEPROM.write(addr + i, deveui[i]);
  };

  // Write appeui
  addr = 40; size = 8;
  for (int i = 0 ; i < size ; i ++) {
    EEPROM.write(addr + i, appeui[i]);
  };

  // Write appkey
  addr = 48; size = 16;
  for (int i = 0 ; i < size ; i ++) {
    EEPROM.write(addr + i, appkey[i]);
  };
};

////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));

  // initialize the LED pin as an output.
  pinMode(13, OUTPUT);

  Serial.print("EEPROM Magic: ");
  Serial.println(magic);

  Serial.print("Version:      ");
  Serial.println(version, DEC);
  Serial.println("");

  // Default Application EUI
  // Devices
  if (device_number == 0) {
    Serial.println(F("No Device Specified, using default values"));
    devname = device.devname;
    memcpy(deveui,  device.deveui,  8);
    memcpy(appeui,  device.appeui,  8);
    memcpy(appkey,  device.appkey,  16);
  } else {
    devname = devices[device_number].devname;
    memcpy(deveui,  devices[device_number].deveui,  8);
    memcpy(appeui,  devices[device_number].appeui,  8);
    memcpy(appkey,  devices[device_number].appkey,  16);
  }

  // Display Configuration//////////////////////////////////
  Serial.print("Device Name:  ");
  Serial.println(device.devname);

  Serial.println("Configuration");
  display_device_configuration(&device);
  
  read_eeprom();

  // turn the LED on when we're done
  digitalWrite(13, HIGH);

  // Display Program / Device Configuration
  Serial.println("Write");
  Serial.print("    Write configuration to EEPROM");
  if (eeprom_write) {
    Serial.println(" [ENABLED]");
    write_eeprom(device,eeprom_erase);
 
    Serial.println("  Read configuration from EEPROM");
    Serial.println();

    Serial.println("Configuration");
    display_device_configuration(&device);
  
  } else {
    Serial.println(" [DISABLED]");
  }
  
};

void loop() {
  digitalWrite(13, HIGH);
  delay(200);
  digitalWrite(13, LOW);
  delay(800);
};
