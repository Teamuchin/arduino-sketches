#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

uint8_t receiverMAC[] = {0x00, 0x70, 0x07, 0x26, 0x9F, 0x28};

#define BUTTON_PIN 25
#define LED_PIN 2  // Built-in LED on most ESP32 boards

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
  uint8_t musicMode; // 0: Mario, 1: Tetris, 2: LoopTrack, 3: Sessiz
} SensorData;

SensorData data;

uint8_t currentMode = 0;

// Variables for debounce logic
bool buttonState = HIGH;       // The current stable reading from the input pin
bool lastButtonState = HIGH;   // The previous raw reading from the input pin
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; 

void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Send Failed");
}

void setup() {
  delay(3000);
  Serial.begin(115200);
  Wire.begin();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT); // Set LED as an output

  // MPU6050 Wake up
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  // 1. Read the state of the switch into a local variable:
  bool reading = digitalRead(BUTTON_PIN);

  // 2. Check to see if you just pressed the button
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      
      // Turn the LED ON when buttonState is LOW (pressed)
      // and OFF when buttonState is HIGH (released)
      digitalWrite(LED_PIN, !buttonState); 

      // only toggle the mode if the new button state is LOW (pressed)
      if (buttonState == LOW) {
        currentMode++;
        if (currentMode > 3) {
          currentMode = 0;
        }
        Serial.print("Yeni Muzik Modu: ");
        Serial.println(currentMode);
      }
    }
  }

  // save the raw reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;

  // --- MPU6050 and ESP-NOW Logic ---
  
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6, true);

  data.x = Wire.read() << 8 | Wire.read();
  data.y = Wire.read() << 8 | Wire.read();
  data.z = Wire.read() << 8 | Wire.read();
  
  data.musicMode = currentMode;

  esp_now_send(receiverMAC, (uint8_t *)&data, sizeof(data));
  delay(100);
}