#include <EEPROM.h>

// Write persistant data to Arduino EEPROM for PAE IoT applications,
// connecting to 'The Things Network' (TTN).
//
// This is useful in that the firmware program can be reflashed,
// but the LoRaWAN node configuration/authentication data is maintained
// on each device.
//
// The source code then does not need to be different for each device.

// If device number is 0, use default settings, otherwise set configuration from device array.
int    device = 2;  // Device number for configuration data

// Software Configuration
int erase_eeprom = 0; // Erase all EEPROM contents.
int write_eeprom = 0; // Write EEPROM, otherwise, just read.
int debug = 0;        // Enable/Disable debug output

int addr = 0;         // Address to read/write data
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
char version  = 0;         // EEPROM Version

// Device Parameters
String devname    = "iot-workshop-1";
byte   deveui[8]  = { 0x01, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 };
byte   appeui[8]  = { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
byte   appkey[16] = { 0x03, 0x7A, 0x7F, 0x9D, 0xED, 0xAA, 0x54, 0xA5, 0xBB, 0x2F, 0xB1, 0x3B, 0x2F, 0x31, 0x68, 0x33 };

// Device structure
typedef struct iot_otaa_dev {
  String  devname;
  byte    deveui[8];
  byte    appeui[8];
  byte    appkey[16];
} iot_otaa_dev;

// Devices ////////////////////////////////////////////////////////////////////////////////////////////
iot_otaa_dev devices[] = {
  { "iot-workshop-0",
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
  },
  { "iot-workshop-1",
    { 0x01, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0x03, 0x7A, 0x7F, 0x9D, 0xED, 0xAA, 0x54, 0xA5, 0xBB, 0x2F, 0xB1, 0x3B, 0x2F, 0x31, 0x68, 0x33 }
  },
  {
    "iot-workshop-2",
    { 0x02, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0xCF, 0x45, 0x0F, 0x3E, 0x92, 0xA6, 0x69, 0x7D, 0x44, 0x95, 0x0D, 0x0A, 0x09, 0x37, 0xDA, 0x4C }
  },
  {
    "iot-workshop-3",
    { 0x03, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0xDB, 0x76, 0xF5, 0x9A, 0xD0, 0x42, 0xF9, 0xBA, 0xB8, 0xC0, 0xD0, 0xE9, 0xB6, 0xBF, 0x01, 0xE9 }
  },
  {
    "iot-workshop-4",
    { 0x04, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0x0B, 0x28, 0xA2, 0x8D, 0xA5, 0x8F, 0xCE, 0x27, 0x21, 0xA4, 0x92, 0xD1, 0x57, 0xC5, 0x39, 0xF0 }
  },
  {
    "iot-workshop-5",
    { 0x05, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0x9D, 0x8E, 0xCD, 0x5A, 0x9B, 0xAA, 0xC3, 0xF7, 0x75, 0xE2, 0x19, 0xA9, 0x83, 0x62, 0xE5, 0xEE }
  },
  {
    "iot-workshop-6",
    { 0x03, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0xDB, 0x76, 0xF5, 0x9A, 0xD0, 0x42, 0xF9, 0xBA, 0xB8, 0xC0, 0xD0, 0xE9, 0xB6, 0xBF, 0x01, 0xE9 }
  },
  {
    "iot-workshop-7",
    { 0x07, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0x01, 0xED, 0xD7, 0x3D, 0xEF, 0x8E, 0x2C, 0x5E, 0x9B, 0x21, 0x7A, 0x24, 0xE4, 0x45, 0xEF, 0x00 }
  },
  {
    "iot-workshop-8",
    { 0x08, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0x95, 0xE8, 0x0C, 0xE1, 0xB3, 0x0B, 0x35, 0x92, 0x2E, 0x55, 0x23, 0xC1, 0xF4, 0xF2, 0x60, 0x89 }
  },
  {
    "iot-workshop-9",
    { 0x09, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0xE4, 0xAE, 0xDC, 0x6D, 0x88, 0x4B, 0xC3, 0x4D, 0xC6, 0x55, 0x98, 0xBD, 0x4F, 0xCC, 0x00, 0x3A }
  },
  {
    "iot-workshop-10",
    { 0x0a, 0x00, 0x71, 0x6F, 0x69, 0x65, 0x61, 0x70 },
    { 0xED, 0xC2, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 },
    { 0x91, 0x49, 0xD6, 0x8F, 0xB5, 0xDC, 0xBF, 0xB1, 0xEC, 0xD2, 0x37, 0x8A, 0xC6, 0xCE, 0x8A, 0xD0 }
  }
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

////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  char tmp[16];
  Serial.begin(115200);
  Serial.println(F("Starting"));

  // initialize the LED pin as an output.
  pinMode(13, OUTPUT);

  // Default Application EUI
  // Devices
  if (device == 0) {
    Serial.println(F("No Device Specified, using default values"));
  } else {
    devname = devices[device].devname;
    memcpy(deveui,  devices[device].deveui,  8);
    memcpy(appeui,  devices[device].appeui,  8);
    memcpy(appkey,  devices[device].appkey,  16);
  }

  // Write EEPROM //////////////////////////////////////////
  if (erase_eeprom) {
    // Erase existing EEPROM
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    };
  };

  if (write_eeprom) {
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

  // Read EEPROM ///////////////////////////////////////////
  Serial.println(F("Reading EEPROM"));

  // magic
  Serial.print("  magic:      ");
  addr = 0; size = 7;
  for (int i = 0; i < size ; i++) {
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
  for (int i = 0; i < size ; i++) {
    devname[i] = EEPROM.read(addr + i);
  }
  Serial.print(devname);
  Serial.println();
  // deveui
  Serial.print("  deveui:     ");
  addr = 32; size = 8;
  for ( int i = 0; i < size; i++) {
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

  Serial.println();

  // turn the LED on when we're done
  digitalWrite(13, HIGH);

  // Display Program / Device Configuration
  Serial.println("Configuration");
  Serial.print("  Write configuration to EEPROM");
  if (write_eeprom) {
    Serial.println(" [ENABLED]");
  } else {
    Serial.println(" [DISABLED]");
  }
  Serial.println("  Read configuration from EEPROM");
  Serial.println();

  Serial.print("Magic:        ");
  Serial.println(magic);

  Serial.print("Version:      ");
  Serial.println(version, DEC);

  Serial.print("Device Name:  ");
  Serial.println(devname);

  Serial.println("Configuration");
  display_device_configuration(&devices[1]);
};

void loop() {
  digitalWrite(13, HIGH);
  delay(200);
  digitalWrite(13, LOW);
  delay(800);
};
