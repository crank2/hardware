#include <SPI.h>
#include <SD.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioTools.h"
#include "AudioESP8266.h"
#include "BluetoothA2DP.h"
#include <EEPROM.h>

#define EEPROM_MAC_ADDRESS_START 0

uint8_t targetMacAddress[6] = {0};
uint8_t macList[10][6]; // Array for up to 10 MAC address
int macListIndex = 0; // Index for MAC address
int currentMacIndex = 0; // Index  MAC address
bool isPlaying = false;
const int sd_ss_pin = 5;
File root;
String currentFileName = "";
BluetoothA2DPSource a2dp_source;
AudioFileSourceSD *file;
AudioGeneratorMP3 *mp3;
AudioOutputWithCallback *out;
int currentTrackIndex = 0;
int totalTracks = 0;
String trackNames[25]; //  SD card
bool wasConnected = false; // 
bool SEARCH = false;
bool useEEPROM = false;
bool isSelectionMade = false; // Status 

// Function declarations
bool isMacAddressStoredInEEPROM();
void writeMacAddressToEEPROM(const uint8_t* macAddress);
void readMacAddressFromEEPROM(uint8_t* macAddress);

bool macAddressExists(uint8_t address[6]) {
  for (int i = 0; i < macListIndex; i++) {
    if (memcmp(macList[i], address, 6) == 0) {
      return true; // MAC address 
    }
  }
  return false; // MAC address
}

bool isValid(const char* ssid, esp_bd_addr_t address, int rssi) {

  
    if (!macAddressExists(address) && macListIndex < 10) { 
      memcpy(macList[macListIndex], address, 6); 
      macListIndex++;
    }
  

  //  target
  if (memcmp(address, targetMacAddress, sizeof(targetMacAddress)) == 0) {
    Serial.print("Found target device with address: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X:", address[i]);
    }
    Serial.println();
    writeMacAddressToEEPROM(targetMacAddress);
    a2dp_source.connect_to(targetMacAddress);
    SEARCH = false;
    return true; // Valid
  }
  return false;
}

// Callback A2DP  data 
int32_t get_sound_data(Channels* data, int32_t len) {
  return out == nullptr ? 0 : out->read(data, len);
}

void loadTrackNames() {
  root = SD.open("/");
  totalTracks = 0;

  // 
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break; // Tidak ada file lagi

    if (!entry.isDirectory()) {
      String fileName = entry.name();
      if (fileName.endsWith(".mp3")) {
        trackNames[totalTracks++] = fileName;
      }
    }
    entry.close();
  }
  root.close();

  
  sortTrackNames();
}


void sortTrackNames() {
  for (int i = 0; i < totalTracks - 1; i++) {
    for (int j = i + 1; j < totalTracks; j++) {
      if (trackNames[i] > trackNames[j]) { 
        String temp = trackNames[i];
        trackNames[i] = trackNames[j];
        trackNames[j] = temp;
      }
    }
  }
}

void listTracks() {
  Serial.println("Available Tracks:");
  for (int i = 0; i < totalTracks; i++) {
    Serial.printf("%d: %s\n", i + 1, trackNames[i].c_str());
  }
}


void playTrack(int index) {
  if (index >= 0 && index < totalTracks) {
    if (mp3->isRunning()) {
      mp3->stop();
    }

    currentFileName = "/" + trackNames[index];
    file = new AudioFileSourceSD();
    if (file->open(currentFileName.c_str())) {
      mp3->begin(file, out);
      Serial.printf("Playing '%s'...\n", currentFileName.c_str());
      isPlaying = true; // Update status
    } else {
      Serial.println("Can't find .mp3 file");
    }
  }
}

void stopTrack() {
  if (mp3->isRunning()) {
    mp3->stop();
    Serial.println("Playback stopped.");
    isPlaying = false; // Update status
  }
}

void nextTrack() {
  currentTrackIndex = (currentTrackIndex + 1) % totalTracks;
  playTrack(currentTrackIndex);
}


void setup(void) {
  Serial.begin(115200);
  EEPROM.begin(512);
  audioLogger = &Serial;

  // Setup Audio
  mp3 = new AudioGeneratorMP3();
  out = new AudioOutputWithCallback(1024, 5);

  // Initialize SD card
  if (!SD.begin(sd_ss_pin)) {
    Serial.println("SD card initialization failed");
    return;
  }

  // Load track names from SD
  loadTrackNames();

  // Initialize pin GPIO
  pinMode(2, INPUT_PULLUP); 
  pinMode(15, INPUT_PULLUP); 
  Serial.println("Press GPIO 2 for EEPROM or GPIO 15 for Search after startup.");
  
  // Initialize A2DP 
  a2dp_source.set_ssid_callback(isValid);
    // Set volume to 50%
  a2dp_source.set_volume(63); // range 0-127
  a2dp_source.start(get_sound_data);
} 

void loop() {
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  }

  bool isConnected = a2dp_source.is_connected();
  if (isConnected && !wasConnected) {
    Serial.println("Bluetooth connected.");
    wasConnected = true;

    playTrack(currentTrackIndex); // 
  } else if (!isConnected && wasConnected) {
    Serial.println("Bluetooth disconnected.");
    wasConnected = false;
  }

  // 
  if (!isSelectionMade) {
    // 
    if (digitalRead(2) == LOW) { // Pin GPIO 2 
      useEEPROM = true;
      Serial.println("EEPROM selected.");
      if (isMacAddressStoredInEEPROM()) {
        readMacAddressFromEEPROM(targetMacAddress);
        Serial.print("Loaded target MAC address from EEPROM: ");
        for (int i = 0; i < 6; i++) {
          Serial.printf("%02X:", targetMacAddress[i]);
        }
        Serial.println();
        isSelectionMade = true; 
      } else {
        Serial.println("No MAC address found in EEPROM.");
      }
    } else if (digitalRead(15) == LOW) { 
      Serial.println("Searching for MAC address...");
      SEARCH = true;
      isSelectionMade = true; 
      


if (macListIndex > 0) {
    Serial.print("Connect MAC address: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X:", macList[0][i]);
    }

  
  } else {
    Serial.print("No MAC addresses Found");

  }    Serial.println();

    }
    delay(200); // Debounce delay
  } 
   else if (SEARCH) {
      if (digitalRead(2) == LOW) {
       
        memcpy(targetMacAddress, macList[currentMacIndex], 6);
        if (useEEPROM) {
          writeMacAddressToEEPROM(targetMacAddress); // 
        }
        Serial.println("Connecting to selected MAC address...");
      } else if (digitalRead(15) == LOW) {
        if(macListIndex>0){
                  currentMacIndex = (currentMacIndex + 1) % macListIndex;
        Serial.print("Next MAC address: ");
        for (int i = 0; i < 6; i++) {
          Serial.printf("%02X:", macList[currentMacIndex][i]);
        }
        }else {
    Serial.print("No MAC addresses Found");
 
  }
        

        Serial.println();
      }    delay(200); // Debounce delay
    }
  else {
    // 
    if (wasConnected) {
      if (digitalRead(2) == LOW) { // Pin GPIO 2 
        if (isPlaying) {
          stopTrack();
        } else {
          playTrack(currentTrackIndex);
        }
        delay(200); // Debounce delay
      } else if (digitalRead(15) == LOW) { // Pin GPIO 15 
        nextTrack();
        delay(200); // Debounce delay
      }
    }
  }
}

// Function EEPROM
void writeMacAddressToEEPROM(const uint8_t* macAddress) {
  for (int i = 0; i < 6; ++i) {
    EEPROM.write(EEPROM_MAC_ADDRESS_START + i, macAddress[i]);
  }
  EEPROM.commit(); 
}

void readMacAddressFromEEPROM(uint8_t* macAddress) {
  for (int i = 0; i < 6; ++i) {
    macAddress[i] = EEPROM.read(EEPROM_MAC_ADDRESS_START + i);
  }
}

bool isMacAddressStoredInEEPROM() {
  for (int i = 0; i < 6; ++i) {
    if (EEPROM.read(EEPROM_MAC_ADDRESS_START + i) != 0xFF) { 
      return true;
    }
  }
  return false;
}
