#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

uint8_t receiverMAC[] = {0x00, 0x70, 0x07, 0x26, 0x9F, 0x28};

#define MUSIC_BUTTON_PIN 25
#define STRAFE_TRIGGER_PIN 32 // <--- NEW: Hold this button to strafe!
#define LED_PIN 2  

// --- BUZZER PINS ---
#define BUZ_1  13
#define BUZ_2  14
#define BUZ_3  26
#define BUZ_4  27

// --- Frekans Haritası ---
#define REST     0
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_DB2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_EB2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_GB2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_AB2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_BB2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_DB3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_EB3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_GB3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_AB3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_BB3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_DB4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_EB4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_GB4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_AB4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_BB4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_DB5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_EB5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_GB5 740
#define NOTE_G5  792
#define NOTE_GS5 831
#define NOTE_AB5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_BB5 932
#define NOTE_B5  988
#define NOTE_C6  1047

struct Note {
  int frequency;
  int duration;
};

// --- Müzik Listeleri ---
Note loopTrack[] = {
  {NOTE_C6,  4}, {NOTE_C2,  8}, {NOTE_C2,  8}, {REST,     4}, {REST,     8}, {NOTE_G2,  8}, {REST,     4}, {REST,     8},
  {NOTE_BB2, 8}, {NOTE_C3,  8}, {REST,     8}, {NOTE_BB2, 8}, {REST,     8}, {NOTE_F2,  8}, {NOTE_EB2, 8}, {REST,     8},
  {NOTE_C2,  8}, {REST,     4}, {REST,     8}, {NOTE_G2,  8}, {REST,     4}, {REST,     8}, {NOTE_BB2, 8},
  {NOTE_C3,  8}, {REST,     8}, {NOTE_BB2, 8}, {REST,     8}, {NOTE_F2,  8}, {NOTE_EB2, 8}, {REST,     8},
  {NOTE_C2,  8}, {REST,     4}, {REST,     8}, {NOTE_G2,  8}, {REST,     4}, {REST,     8}, {NOTE_BB2, 8},
  {NOTE_C3,  8}, {REST,     8}, {NOTE_BB2, 8}, {REST,     8}, {NOTE_F2,  8}, {NOTE_EB2, 8}, {REST,     8},
  {NOTE_C4,  2}, {REST,     8}, {NOTE_E4,  2}, {REST,     8}, {NOTE_G4,  2}, {NOTE_A4,  2}, {NOTE_BB4, 4},
  {NOTE_C5,  8}, {REST,     8}, {NOTE_C5,  8}, {REST,     8}, {NOTE_BB4, 8}, {REST,     8}, {NOTE_A4,  8}, {REST,     8},
  {NOTE_BB4, 8}, {NOTE_C3,  8}, {NOTE_BB4, 8}, {NOTE_C5,  8}, {REST,     8}, {NOTE_BB4, 8}, {NOTE_A4,  8}, {REST,     8},
  {NOTE_C2,  8}, {NOTE_C5,  8}, {REST,     8}, {NOTE_BB4, 8}, {REST,     8}, {NOTE_A4,  8}, {REST,     8}, {NOTE_BB4, 8},
  {NOTE_C3,  8}, {REST,     8}, {NOTE_BB2, 8}, {NOTE_E5,  8}, {NOTE_F2,  8}, {NOTE_EB2, 8}, {REST,     8}, {NOTE_C2,  8},
  {NOTE_C5,  8}, {REST,     8}, {NOTE_BB4, 8}, {NOTE_G2,  8}, {NOTE_A4,  8}, {REST,     8}, {NOTE_BB4, 8}, {NOTE_BB2, 8},
  {NOTE_BB4, 8}, {NOTE_C5,  8}, {NOTE_BB2, 8}, {NOTE_BB4, 8}, {NOTE_A4,  8}, {NOTE_EB2, 8}, {REST,     8}, {NOTE_C5,  8},
  {REST,     4}, {NOTE_BB4, 8}, {NOTE_G2,  8}, {NOTE_A4,  8}, {REST,     8}, {NOTE_BB4, 8}, {NOTE_BB2, 8}, {NOTE_C3,  8},
  {REST,     8}, {NOTE_E4,  8}, {REST,     8}, {NOTE_F2,  8}, {NOTE_C4,  8}, {REST,     8}, {NOTE_C4,  8}, {REST,     8},
  {NOTE_BB4, 8}, {REST,     8}, {NOTE_A4,  8}, {REST,     8}, {NOTE_BB4, 8}, {REST,     8}, {NOTE_BB4, 8}, {NOTE_C5,  8},
  {REST,     8}, {NOTE_BB4, 8}, {NOTE_A4,  8}, {NOTE_F2,  8}, {NOTE_EB2, 8}, {NOTE_C5,  8}, {NOTE_C2,  8}, {REST,     8},
  {NOTE_BB4, 8}, {REST,     8}, {NOTE_A4,  8}, {REST,     8}, {NOTE_BB4, 8}, {REST,     8}, {NOTE_BB2, 8}, {NOTE_E5,  8},
  {REST,     8}, {NOTE_F2,  8}, {NOTE_C5,  8}, {REST,     8}, {NOTE_BB4, 8}, {REST,     8}, {NOTE_A4,  8}, {REST,     8},
  {NOTE_BB4, 8}, {REST,     8}, {NOTE_BB4, 8}, {NOTE_C5,  8}, {NOTE_BB2, 8}, {NOTE_BB4, 8}, {NOTE_A4,  8}, {REST,     4},
  {NOTE_C5,  8}, {REST,     4}, {NOTE_BB4, 8}, {REST,     8}, {NOTE_A4,  8}, {REST,     8}, {NOTE_BB4, 8}, {REST,     4},
  {REST,     8}, {NOTE_E4,  8}, {NOTE_E4,  8}, {NOTE_E4,  8}, {REST,     8}, {NOTE_AB2, 8}, {REST,     8}, {NOTE_AB2, 8}, {REST,     8},
  {NOTE_C4,  1}, {NOTE_EB4, 1}, {NOTE_F4,  2}, {NOTE_G4,  1}, {NOTE_BB4, 2}, {NOTE_C5,  2}, {NOTE_C6,  2}, {NOTE_C4,  1},
  {NOTE_EB4, 1}, {NOTE_F4,  2}, {NOTE_G4,  1}, {NOTE_BB4, 2}, {NOTE_C5,  2}, {NOTE_C6,  2}, {NOTE_BB2, 8}, {REST,     8},
  {NOTE_BB2, 8}, {REST,     8}, {NOTE_G4,  2}, {NOTE_BB4, 4}, {NOTE_C5,  4}, {NOTE_G4,  1}, {NOTE_EB4, 2}, {NOTE_F4,  2}, {NOTE_C6,  2}
};

Note tetrisTrack[] = {
  {NOTE_E5,  4}, {NOTE_B4,  8}, {NOTE_C5,  8}, {NOTE_D5,  8}, {NOTE_E5,  16}, {NOTE_D5, 16}, {NOTE_C5,  8}, {NOTE_B4,  8},
  {NOTE_A4,  4}, {NOTE_A4,  8}, {NOTE_C5,  8}, {NOTE_E5,  4}, {NOTE_D5,  8}, {NOTE_C5,  8},
  {NOTE_B4,  3}, {NOTE_C5,  8}, {NOTE_D5,  4}, {NOTE_E5,  4},
  {NOTE_C5,  4}, {NOTE_A4,  4}, {NOTE_A4,  2},
  {REST,     8}, {NOTE_D5,  4}, {NOTE_F5,  8}, {NOTE_A5,  4}, {NOTE_G5,  8}, {NOTE_F5,  8},
  {NOTE_E5,  3}, {NOTE_C5,  8}, {NOTE_E5,  4}, {NOTE_D5,  8}, {NOTE_C5,  8},
  {NOTE_B4,  4}, {NOTE_B4,  8}, {NOTE_C5,  8}, {NOTE_D5,  4}, {NOTE_E5,  4},
  {NOTE_C5,  4}, {NOTE_A4,  4}, {NOTE_A4,  4}, {REST,     4},
  {NOTE_E5,  4}, {NOTE_B4,  8}, {NOTE_C5,  8}, {NOTE_D5,  8}, {NOTE_E5,  16}, {NOTE_D5, 16}, {NOTE_C5,  8}, {NOTE_B4,  8},
  {NOTE_A4,  4}, {NOTE_A4,  8}, {NOTE_C5,  8}, {NOTE_E5,  4}, {NOTE_D5,  8}, {NOTE_C5,  8},
  {NOTE_B4,  3}, {NOTE_C5,  8}, {NOTE_D5,  4}, {NOTE_E5,  4},
  {NOTE_C5,  4}, {NOTE_A4,  4}, {NOTE_A4,  2},
  {REST,     8}, {NOTE_D5,  4}, {NOTE_F5,  8}, {NOTE_A5,  4}, {NOTE_G5,  8}, {NOTE_F5,  8},
  {NOTE_E5,  3}, {NOTE_C5,  8}, {NOTE_E5,  4}, {NOTE_D5,  8}, {NOTE_C5,  8},
  {NOTE_B4,  4}, {NOTE_B4,  8}, {NOTE_C5,  8}, {NOTE_D5,  4}, {NOTE_E5,  4},
  {NOTE_C5,  4}, {NOTE_A4,  4}, {NOTE_A4,  4}, {REST,     4},
  {NOTE_E4,  2}, {NOTE_C4,  2}, {NOTE_D4,  2}, {NOTE_B3,  2}, {NOTE_C4,  2}, {NOTE_A3,  2},
  {NOTE_GS3, 2}, {NOTE_B3,  2}, {NOTE_E4,  2}, {NOTE_C4,  2}, {NOTE_D4,  2}, {NOTE_B3,  2},
  {NOTE_C4,  4}, {NOTE_E4,  4}, {NOTE_C5,  2}, {NOTE_GS4, 1}
};

Note marioTrack[] = {
  {NOTE_E5, 16}, {NOTE_E5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_E5, 8}, {NOTE_G5, 4}, {NOTE_G4, 4},
  {NOTE_C5, 8}, {REST, 16}, {NOTE_G4, 16}, {REST, 8}, {NOTE_E4, 8}, {REST, 16}, {NOTE_A4, 16}, {REST, 16}, {NOTE_B4, 16}, {REST, 16}, {NOTE_AS4, 16}, {NOTE_A4, 8},
  {NOTE_G4, 12}, {NOTE_E5, 12}, {NOTE_G5, 12}, {NOTE_A5, 8}, {NOTE_F5, 16}, {NOTE_G5, 16}, {REST, 16}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 16}, {NOTE_B4, 16}, {REST, 8},
  {NOTE_C5, 8}, {REST, 16}, {NOTE_G4, 16}, {REST, 8}, {NOTE_E4, 8}, {REST, 16}, {NOTE_A4, 16}, {REST, 16}, {NOTE_B4, 16}, {REST, 16}, {NOTE_AS4, 16}, {NOTE_A4, 8},
  {NOTE_G4, 12}, {NOTE_E5, 12}, {NOTE_G5, 12}, {NOTE_A5, 8}, {NOTE_F5, 16}, {NOTE_G5, 16}, {REST, 16}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 16}, {NOTE_B4, 16}, {REST, 8},
  {REST, 8}, {NOTE_G5, 16}, {NOTE_FS5, 16}, {NOTE_F5, 16}, {NOTE_DS5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_GS4, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {REST, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {NOTE_D5, 16},
  {REST, 8}, {NOTE_G5, 16}, {NOTE_FS5, 16}, {NOTE_F5, 16}, {NOTE_DS5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C6, 16}, {REST, 16}, {NOTE_C6, 16}, {NOTE_C6, 4},
  {REST, 8}, {NOTE_G5, 16}, {NOTE_FS5, 16}, {NOTE_F5, 16}, {NOTE_DS5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_GS4, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {REST, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {NOTE_D5, 16},
  {REST, 8}, {NOTE_DS5, 8}, {REST, 16}, {NOTE_D5, 16}, {REST, 8}, {NOTE_C5, 4}, {REST, 4},
  {REST, 8}, {NOTE_G5, 16}, {NOTE_FS5, 16}, {NOTE_F5, 16}, {NOTE_DS5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_GS4, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {REST, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {NOTE_D5, 16},
  {REST, 8}, {NOTE_G5, 16}, {NOTE_FS5, 16}, {NOTE_F5, 16}, {NOTE_DS5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C6, 16}, {REST, 16}, {NOTE_C6, 16}, {NOTE_C6, 4},
  {REST, 8}, {NOTE_G5, 16}, {NOTE_FS5, 16}, {NOTE_F5, 16}, {NOTE_DS5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_GS4, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {REST, 16}, {NOTE_A4, 16}, {NOTE_C5, 16}, {NOTE_D5, 16},
  {REST, 8}, {NOTE_DS5, 8}, {REST, 16}, {NOTE_D5, 16}, {REST, 8}, {NOTE_C5, 4}, {REST, 4},
  {NOTE_C5, 16}, {NOTE_C5, 8}, {NOTE_C5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 8}, {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_A4, 16}, {NOTE_G4, 4},
  {NOTE_C5, 16}, {NOTE_C5, 8}, {NOTE_C5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 16}, {NOTE_E5, 16}, {REST, 2},
  {NOTE_C5, 16}, {NOTE_C5, 8}, {NOTE_C5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 8}, {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_A4, 16}, {NOTE_G4, 4},
  {NOTE_E5, 16}, {NOTE_E5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_E5, 8}, {NOTE_G5, 4}, {NOTE_G4, 4},
  {NOTE_C5, 8}, {REST, 16}, {NOTE_G4, 16}, {REST, 8}, {NOTE_E4, 8}, {REST, 16}, {NOTE_A4, 16}, {REST, 16}, {NOTE_B4, 16}, {REST, 16}, {NOTE_AS4, 16}, {NOTE_A4, 8},
  {NOTE_G4, 12}, {NOTE_E5, 12}, {NOTE_G5, 12}, {NOTE_A5, 8}, {NOTE_F5, 16}, {NOTE_G5, 16}, {REST, 16}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 16}, {NOTE_B4, 16}, {REST, 8},
  {NOTE_C5, 8}, {REST, 16}, {NOTE_G4, 16}, {REST, 8}, {NOTE_E4, 8}, {REST, 16}, {NOTE_A4, 16}, {REST, 16}, {NOTE_B4, 16}, {REST, 16}, {NOTE_AS4, 16}, {NOTE_A4, 8},
  {NOTE_G4, 12}, {NOTE_E5, 12}, {NOTE_G5, 12}, {NOTE_A5, 8}, {NOTE_F5, 16}, {NOTE_G5, 16}, {REST, 16}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 16}, {NOTE_B4, 16}, {REST, 8},
  {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_G4, 16}, {REST, 8}, {NOTE_GS4, 8}, {NOTE_A4, 16}, {NOTE_F5, 8}, {REST, 32}, {NOTE_F5, 16}, {NOTE_A4, 4},
  {NOTE_D5, 12}, {NOTE_A5, 12}, {NOTE_A5, 12}, {NOTE_A5, 12}, {NOTE_G5, 12}, {NOTE_F5, 12}, {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_A4, 16}, {NOTE_G4, 4},
  {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_G4, 16}, {REST, 8}, {NOTE_GS4, 8}, {NOTE_A4, 16}, {NOTE_F5, 8}, {REST, 32}, {NOTE_F5, 16}, {NOTE_A4, 4},
  {NOTE_D5, 16}, {NOTE_F5, 8}, {NOTE_F5, 16}, {NOTE_F5, 12}, {NOTE_E5, 12}, {NOTE_D5, 12}, {NOTE_C5, 16}, {NOTE_G4, 8}, {NOTE_G4, 16}, {NOTE_C4, 4},
  {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_G4, 16}, {REST, 8}, {NOTE_GS4, 8}, {NOTE_A4, 16}, {NOTE_F5, 8}, {REST, 32}, {NOTE_F5, 16}, {NOTE_A4, 4},
  {NOTE_D5, 12}, {NOTE_A5, 12}, {NOTE_A5, 12}, {NOTE_A5, 12}, {NOTE_G5, 12}, {NOTE_F5, 12}, {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_A4, 16}, {NOTE_G4, 4},
  {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_G4, 16}, {REST, 8}, {NOTE_GS4, 8}, {NOTE_A4, 16}, {NOTE_F5, 8}, {REST, 32}, {NOTE_F5, 16}, {NOTE_A4, 4},
  {NOTE_D5, 16}, {NOTE_F5, 8}, {NOTE_F5, 16}, {NOTE_F5, 12}, {NOTE_E5, 12}, {NOTE_D5, 12}, {NOTE_C5, 16}, {NOTE_G4, 8}, {NOTE_G4, 16}, {NOTE_C4, 4},
  {NOTE_C5, 16}, {NOTE_C5, 8}, {NOTE_C5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 8}, {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_A4, 16}, {NOTE_G4, 4},
  {NOTE_C5, 16}, {NOTE_C5, 8}, {NOTE_C5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 16}, {NOTE_E5, 16}, {REST, 2},
  {NOTE_C5, 16}, {NOTE_C5, 8}, {NOTE_C5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_D5, 8}, {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_A4, 16}, {NOTE_G4, 4},
  {NOTE_E5, 16}, {NOTE_E5, 8}, {NOTE_E5, 16}, {REST, 16}, {NOTE_C5, 16}, {NOTE_E5, 8}, {NOTE_G5, 4}, {NOTE_G4, 4},
  {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_G4, 16}, {REST, 8}, {NOTE_GS4, 8}, {NOTE_A4, 16}, {NOTE_F5, 8}, {REST, 32}, {NOTE_F5, 16}, {NOTE_A4, 4},
  {NOTE_D5, 12}, {NOTE_A5, 12}, {NOTE_A5, 12}, {NOTE_A5, 12}, {NOTE_G5, 12}, {NOTE_F5, 12}, {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_A4, 16}, {NOTE_G4, 4},
  {NOTE_E5, 16}, {NOTE_C5, 8}, {NOTE_G4, 16}, {REST, 8}, {NOTE_GS4, 8}, {NOTE_A4, 16}, {NOTE_F5, 8}, {REST, 32}, {NOTE_F5, 16}, {NOTE_A4, 4},
  {NOTE_D5, 16}, {NOTE_F5, 8}, {NOTE_F5, 16}, {NOTE_F5, 12}, {NOTE_E5, 12}, {NOTE_D5, 12}, {NOTE_C5, 16}, {NOTE_G4, 8}, {NOTE_G4, 16}, {NOTE_C4, 4},
  {NOTE_C5, 6}, {NOTE_G4, 6}, {NOTE_E4, 8}, {NOTE_A4, 12}, {NOTE_B4, 12}, {NOTE_A4, 12}, {NOTE_GS4, 12}, {NOTE_AS4, 12}, {NOTE_A4, 12},
  {NOTE_G4, 1}
};

int loopCount   = 128; 
int tetrisCount = 85;
int marioCount  = 200;

int SPEED_DIVIDER = 900;
int lastTrackMode = -1;    
int noteIndex = 0;            
unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
bool isNotePlaying = false;

// --- ESP-NOW STRUCT ---
typedef struct {
  int16_t accelX; // Roll (Hand tilt left/right)
  int16_t accelY; // Pitch (Hand tilt forward/back)
  int16_t accelZ; 
  int16_t gyroZ;  // Not used for movement anymore, but kept for struct alignment
  uint8_t musicMode; 
  uint8_t triggerHeld; // <--- NEW: Tells receiver if the strafe button is pressed
} SensorData;

SensorData data;
uint8_t currentMode = 0;

// Debounce logic for Music Button
bool buttonState = HIGH;       
bool lastButtonState = HIGH;   
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 

void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Send Failed");
}

void stopBuzzer() {
  ledcWriteTone(BUZ_1, 0);
  ledcWriteTone(BUZ_2, 0);
  ledcWriteTone(BUZ_3, 0);
  ledcWriteTone(BUZ_4, 0);
}

void updateMusicAsync() {
  unsigned long currentMillis = millis();

  if (currentMode != lastTrackMode) {
    stopBuzzer();
    noteIndex = 0;
    isNotePlaying = false;
    lastTrackMode = currentMode;
  }

  if (currentMode == 3) {
    stopBuzzer();
    return;
  }

  if (isNotePlaying && (currentMillis - noteStartTime < noteDuration)) return; 

  isNotePlaying = false;
  Note currentNote;
  int maxNotes = 0;
  float durationMultiplier = 1.0;

  if (currentMode == 0) {
    maxNotes = marioCount;
    currentNote = marioTrack[noteIndex];
    durationMultiplier = 2.5;
  } else if (currentMode == 1) {
    maxNotes = tetrisCount;
    currentNote = tetrisTrack[noteIndex];
    durationMultiplier = 1.6;
  } else if (currentMode == 2) {
    maxNotes = loopCount;
    currentNote = loopTrack[noteIndex];
    durationMultiplier = 1.0;
  }

  noteDuration = (SPEED_DIVIDER / currentNote.duration) * durationMultiplier;
  noteStartTime = currentMillis;
  isNotePlaying = true;

  if (currentNote.frequency != REST) {
    ledcWriteTone(BUZ_1, currentNote.frequency);
    ledcWriteTone(BUZ_2, currentNote.frequency + 4);
    ledcWriteTone(BUZ_3, currentNote.frequency + 2);
    ledcWriteTone(BUZ_4, currentNote.frequency - 2); 
  } else {
    stopBuzzer();
  }

  noteIndex++;
  if (noteIndex >= maxNotes) noteIndex = 0; 
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  Wire.begin();

  pinMode(MUSIC_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STRAFE_TRIGGER_PIN, INPUT_PULLUP); // New Trigger Pin
  pinMode(LED_PIN, OUTPUT); 

  ledcAttach(BUZ_1, 2000, 8);
  ledcAttach(BUZ_2, 2000, 8);
  ledcAttach(BUZ_3, 2000, 8);
  ledcAttach(BUZ_4, 2000, 8); 

  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_now_init();
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  updateMusicAsync();

  // 1. Music Button Logic
  bool reading = digitalRead(MUSIC_BUTTON_PIN);
  if (reading != lastButtonState) lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      digitalWrite(LED_PIN, !buttonState); 
      if (buttonState == LOW) {
        currentMode++;
        if (currentMode > 3) currentMode = 0;
      }
    }
  }
  lastButtonState = reading;

  // 2. Read Strafe Trigger (LOW means pressed)
  data.triggerHeld = (digitalRead(STRAFE_TRIGGER_PIN) == 0) ? 1 : 0;

  // 3. Sensor & Transmit (Every 50ms)
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime >= 50) {
    lastSendTime = millis();
    
    Wire.beginTransmission(0x68);
    Wire.write(0x3B); 
    Wire.endTransmission(false);
    Wire.requestFrom((uint16_t)0x68, (uint8_t)14, true); 

    data.accelX = Wire.read() << 8 | Wire.read();
    data.accelY = Wire.read() << 8 | Wire.read();
    data.accelZ = Wire.read() << 8 | Wire.read();
    
    Wire.read(); Wire.read(); 
    Wire.read(); Wire.read(); 
    Wire.read(); Wire.read(); 
    
    data.gyroZ = Wire.read() << 8 | Wire.read(); 
    data.musicMode = currentMode;

    esp_now_send(receiverMAC, (uint8_t *)&data, sizeof(data));
  }
}