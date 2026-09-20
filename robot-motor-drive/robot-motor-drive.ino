#include <WiFi.h>
#include <esp_now.h>

// --- L298N Motor Sürücü Pin Tanımlamaları ---
#define INA 27
#define INB 26
#define INC 25
#define IND 33
#define ENB 12
#define ENA 14

#define INA2 18
#define INB2 19
#define INC2 21
#define IND2 22
#define ENB2 32
#define ENA2 23


int SPEED_DIVIDER = 900;         

unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
bool isNotePlaying = false;

// --- DİNAMİK İVMELENME YAPILANDIRMASI ---
const float MIN_ACCEL = 0.2;   
const float MAX_ACCEL = 5.0;   
const float DECEL_STEP = 30.0; 

float currentSpeedLeft = 0;
float currentSpeedRight = 0;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t z;
  uint8_t musicMode; 
} SensorData;

SensorData data;
unsigned long lastMotorUpdateTime = 0;

void onReceive(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&data, incomingData, sizeof(data));
}

float rampSpeedDynamic(float current, float target, float dynamicAccel) {
  if (current < target) {
    current += dynamicAccel;
    if (current > target) current = target;
  } else if (current > target) {
    current -= DECEL_STEP;
    if (current < target) current = target;
  }
  return current;
}

float applyExponentCurve(int16_t input, float deadzone, float maxVal, float outMax, float exponent) {
  float absInput = abs(input);
  if (absInput < deadzone) return 0;
  
  float normalized = (absInput - deadzone) / (maxVal - deadzone);
  normalized = constrain(normalized, 0.0, 1.0);
  
  float output = outMax * pow(normalized, exponent);
  return (input > 0) ? output : -output;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // Motor Pinleri
  pinMode(INA, OUTPUT); pinMode(INB, OUTPUT); pinMode(INC, OUTPUT); pinMode(IND, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(INC2, OUTPUT); pinMode(IND2, OUTPUT);
  pinMode(ENB2, OUTPUT); pinMode(ENA2, OUTPUT);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onReceive);
}

void loop() {

  // 2. MOTOR HAREKET KONTROLÜ (20ms Ritim)
  if (millis() - lastMotorUpdateTime >= 20) {
    lastMotorUpdateTime = millis();

    float absX = abs(data.x);
    float absY = abs(data.y);
    float maxInput = max(absX, absY); 
    
    float tiltRatio = 0.0;
    if (maxInput > 4000) {
      tiltRatio = (maxInput - 4000) / (16000 - 4000);
      tiltRatio = constrain(tiltRatio, 0.0, 1.0);
    }

    float dynamicMaxForward = 255 * tiltRatio;
    float dynamicMaxTurn = 120 * tiltRatio;
    float currentAccelStep = MIN_ACCEL + (tiltRatio * (MAX_ACCEL - MIN_ACCEL));

    int forwardSpeed = applyExponentCurve(-data.y, 4000, 16000, dynamicMaxForward, 0.7);
    int turnSpeed = applyExponentCurve(-data.x, 4000, 16000, dynamicMaxTurn, 0.5); 

    int targetLeft = forwardSpeed + turnSpeed;
    int targetRight = forwardSpeed - turnSpeed;

    targetLeft = constrain(targetLeft, -255, 255);
    targetRight = constrain(targetRight, -255, 255);

    int targetSpeedLeft1 = 0;

    if (targetLeft > 0) {
      digitalWrite(INA, LOW);  digitalWrite(INB, HIGH);
      digitalWrite(INA2, LOW); digitalWrite(INB2, HIGH);
      targetSpeedLeft1 = targetLeft;
    } else if (targetLeft < 0) {
      digitalWrite(INA, HIGH);  digitalWrite(INB, LOW);
      digitalWrite(INA2, HIGH); digitalWrite(INB2, LOW);
      targetSpeedLeft1 = abs(targetLeft);
    } else {
      digitalWrite(INA, LOW);  digitalWrite(INB, LOW);
      digitalWrite(INA2, LOW); digitalWrite(INB2, LOW);
    }

    int targetSpeedRight1 = 0;

    if (targetRight > 0) {
      digitalWrite(INC, HIGH); digitalWrite(IND, LOW);
      digitalWrite(INC2, HIGH); digitalWrite(IND2, LOW);
      targetSpeedRight1 = targetRight;
    } else if (targetRight < 0) {
      digitalWrite(INC, LOW);  digitalWrite(IND, HIGH);
      digitalWrite(INC2, LOW);  digitalWrite(IND2, HIGH);
      targetSpeedRight1 = abs(targetRight);
    } else {
      digitalWrite(INC, LOW);  digitalWrite(IND, LOW);
      digitalWrite(INC2, LOW); digitalWrite(IND2, LOW);
    }

    currentSpeedLeft = rampSpeedDynamic(currentSpeedLeft, targetSpeedLeft1, currentAccelStep);
    currentSpeedRight = rampSpeedDynamic(currentSpeedRight, targetSpeedRight1, currentAccelStep);

    float currentSpeedLeft2 = constrain(currentSpeedLeft * 1.5, 0, 255);
    float currentSpeedRight2 = constrain(currentSpeedRight * 1.5, 0, 255);

    analogWrite(ENA, (int)currentSpeedLeft);
    analogWrite(ENB, (int)currentSpeedRight);
    analogWrite(ENA2, (int)currentSpeedLeft2);
    analogWrite(ENB2, (int)currentSpeedRight2);
  }
}