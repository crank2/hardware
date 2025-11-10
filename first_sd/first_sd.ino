#include <Arduino_BuiltIn.h>


//#include <AudioTools.h>
//#include "AudioTools/AudioLibs/A2DPStream.h"
//#include "AudioTools/Disk/AudioSourceSDFAT.h"
//#include "AudioTools/Audi
#include <BluetoothSerial.h>
#include <BluetoothA2DPSource.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
 
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif


#include <FS.h>
#include <SD.h>
#include <SPI.h>

//Modify the corresponding pin according to the circuit diagram.
#define SD_MOSI 19
#define SD_MISO 21
#define SD_SCK 5
#define SD_CS 33
BluetoothA2DPSource a2dp_source;
File sound_file;
const char* file_name = "/audio.raw";
//const int sd_ss_pin = 5;
const int frame_size_bytes = sizeof(int16_t) * 2;

SPIClass SD_SPI;
BluetoothSerial SerialBT;
 
#define BT_DISCOVER_TIME 10000
 
static bool btScanAsync = true;
static bool btScanSync = false;

int led = LED_BUILTIN;

void setup() {
  // Some boards work best if we also make a serial connection
  Serial.begin(115200);
  if (sd_init() == 0)
  {
    Serial.println("TF init success");
  } else {
    Serial.println("TF init fail");
  }

  // set LED to be an output pin
  pinMode(led, OUTPUT);
  SerialBT.begin("ESP32test");  //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
 
  if (btScanAsync) {
    Serial.print("Starting asynchronous discovery... ");
    if (SerialBT.discoverAsync(btAdvertisedDeviceFound)) {
      Serial.println("Findings will be reported in \"btAdvertisedDeviceFound\"");
      delay(10000);
 //     Serial.printf("Found a device asynchronously: %s\n", pDevice->toString().c_str());
      Serial.print("Stopping discoverAsync... ");
      SerialBT.discoverAsyncStop();
      Serial.println("stopped");
    } else {
      Serial.println("Error on discoverAsync f.e. not working after a \"connect\"");
    }
  }
 
  if (btScanSync) {
    Serial.println("Starting synchronous discovery... ");
    BTScanResults *pResults = SerialBT.discover(BT_DISCOVER_TIME);
    if (pResults) {
      pResults->dump(&Serial);
    } else {
      Serial.println("Error on BT Scan, no result!");
    }
  }
}
/*
// callback used by A2DP to provide the sound data
int32_t get_sound_data(Channels* data, int32_t len) {
  // the data in the file must be in int16 with 2 channels 
  size_t result_len_bytes = sound_file.read((uint8_t*)data, len * frame_size_bytes );
  // result is in number of frames
  int32_t result_len = result_len_bytes / frame_size_bytes;
  ESP_LOGD("get_sound_data", "%d -> %d",len );
  return result_len;
}
*/ 
void btAdvertisedDeviceFound(BTAdvertisedDevice *pDevice) {
  Serial.printf("in btAdvertise");
  Serial.printf("Found a device asynchronously: %s\n", pDevice->toString().c_str());
}
 


void loop() {
  // Say hi!
  Serial.println("Loop!");
  setup();
  while(1) {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                // wait for a half second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                // wait for a half second
  }
}


/******************************************************************************/


// SD test
void listDir(fs::FS & fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}


int sd_init()
{
  //SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  Serial.println("sd_init");
   SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SD_SPI, 80000000))
  {
    Serial.println("Card Mount Failed");
    return 1;
  }
  else
  {
    Serial.println("Card Mount Successed");
  }
  listDir(SD, "/", 0);
  Serial.println("TF Card init over.");
  return 0;
}


uint16_t read16(File& f) {
  uint16_t result;
  ((uint8_t*)&result)[0] = f.read();
  ((uint8_t*)&result)[1] = f.read();
  return result;
}

uint32_t read32(File& f) {
  uint32_t result;
  ((uint8_t*)&result)[0] = f.read();
  ((uint8_t*)&result)[1] = f.read();
  ((uint8_t*)&result)[2] = f.read();
  ((uint8_t*)&result)[3] = f.read();
  return result;
}

/*
#include "BluetoothA2DPSource.h"
#include <SPI.h>
#include <SD.h>

BluetoothA2DPSource a2dp_source;
File sound_file;
const char* file_name = "/audio.raw";
const int sd_ss_pin = 5;
const int frame_size_bytes = sizeof(int16_t) * 2;

// callback used by A2DP to provide the sound data
int32_t get_sound_data(Channels* data, int32_t len) {
  // the data in the file must be in int16 with 2 channels 
  size_t result_len_bytes = sound_file.read((uint8_t*)data, len * frame_size_bytes );
  // result is in number of frames
  int32_t result_len = result_len_bytes / frame_size_bytes;
  ESP_LOGD("get_sound_data", "%d -> %d",len );
  return result_len;
}

// Arduino Setup
void setup(void) {
  Serial.begin(115200);

  // Setup SD and open file
  SD.begin(sd_ss_pin);
  sound_file = SD.open(file_name, FILE_READ);

  // start the bluetooth
  Serial.println("starting A2DP...");
  a2dp_source.start("MyMusic", get_sound_data);  
}

// Arduino loop - repeated processing 
void loop() {
}
*/