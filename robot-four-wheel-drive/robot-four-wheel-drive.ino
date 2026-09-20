#include <WiFi.h>
#include <esp_now.h>
#include <math.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// --- L298N Motor Pins ---
#define INA 27
#define INB 26
#define ENA 14 // FRONT LEFT
#define INC 25
#define IND 33
#define ENB 12 // FRONT RIGHT

#define INA2 18
#define INB2 19
#define ENA2 23 // BACK LEFT
#define INC2 21
#define IND2 22
#define ENB2 32 // BACK RIGHT

// --- DOT MATRIX SETTINGS ---
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW 
#define MAX_DEVICES 4                     
#define CLK_PIN   13 
#define DATA_PIN  15 
#define CS_PIN    5

MD_Parola botDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// ==========================================
// THE ORB EYES
// ==========================================
const uint8_t eyeOpen[8] = { 0b00111100, 0b01111110, 0b11100111, 0b11100111, 0b11111111, 0b11111111, 0b01111110, 0b00111100 };
const uint8_t eyeLeft[8] = { 0b00111100, 0b01111110, 0b11001111, 0b11001111, 0b11111111, 0b11111111, 0b01111110, 0b00111100 };
const uint8_t eyeRight[8] = { 0b00111100, 0b01111110, 0b11110011, 0b11110011, 0b11111111, 0b11111111, 0b01111110, 0b00111100 };
const uint8_t eyeUp[8]   = { 0b00111100, 0b01100110, 0b11100111, 0b11111111, 0b11111111, 0b11111111, 0b01111110, 0b00111100 };
const uint8_t eyeDown[8] = { 0b00111100, 0b01111110, 0b11111111, 0b11111111, 0b11100111, 0b11100111, 0b01111110, 0b00111100 };
const uint8_t eyeHalf[8] = { 0b00000000, 0b00011000, 0b00111100, 0b00111100, 0b01111110, 0b01111110, 0b00111100, 0b00011000 };
const uint8_t eyeClosed[8] = { 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00000000, 0b00000000 };

const uint8_t* currentEyeFrame = eyeOpen; 

// ==========================================
// THE ZINGERS
// ==========================================
const char* zingers[] = {
  ">:) KACHOW! ",
  " GET TO THE CHOPPA!!",
  "LUDICROUS SPEED! XD",
  "I AM SPEED. ಠ_ಠ",
  "[ -_-]~ OUTTA MY WAY"
};
const int numZingers = 5;

// Matrix Timers
int globalMode = 0; 
unsigned long lastModeSwitch = 0;
unsigned long modeDuration = 35000; 
unsigned long lastLookTime = 0;
unsigned long nextLookDelay = 1500;
int blinkStep = 0; 
unsigned long lastBlinkTime = 0;
unsigned long nextBlinkDelay = 3000; 

// Motor Variables
const float MIN_ACCEL = 0.5;   
const float MAX_ACCEL = 5.0;   
const float DECEL_STEP = 30.0; 
float curSpeedFL = 0, curSpeedFR = 0, curSpeedBL = 0, curSpeedBR = 0;

// ESP-NOW Struct
typedef struct {
  int16_t accelX; 
  int16_t accelY; 
  int16_t accelZ; 
  int16_t gyroZ;  
  uint8_t musicMode; 
  uint8_t triggerHeld; 
} SensorData;

SensorData data;
unsigned long lastMotorUpdateTime = 0;

void onReceive(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&data, incomingData, sizeof(data));
}

void drawFace(const uint8_t* frame) {
  MD_MAX72XX *mx = botDisplay.getGraphicObject();
  mx->control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF); 
  mx->clear();
  for(int i=0; i<8; i++) {
    mx->setColumn(20 + i, frame[i]); 
    mx->setColumn(4 + i, frame[i]);  
  }
  mx->control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON); 
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
  float normalized = constrain((absInput - deadzone) / (maxVal - deadzone), 0.0, 1.0);
  float output = outMax * pow(normalized, exponent);
  return (input > 0) ? output : -output;
}

void driveMotor(int targetSpeed, int pinA, int pinB, int pwmPin, float &currentSpeed, float accelStep) {
  int targetMagnitude = 0;

  // 1. INSTANT DIRECTION AND BRAKING (Mimicking your original V1 logic)
  if (targetSpeed > 0) {
    digitalWrite(pinA, LOW); digitalWrite(pinB, HIGH);
    targetMagnitude = targetSpeed;
  } else if (targetSpeed < 0) {
    digitalWrite(pinA, HIGH); digitalWrite(pinB, LOW);
    targetMagnitude = abs(targetSpeed);
  } else {
    // INSTANT BRAKE: Bypasses deceleration entirely just like V1
    digitalWrite(pinA, LOW); digitalWrite(pinB, LOW); 
    targetMagnitude = 0;
  }

  // 2. RAMP MAGNITUDE
  currentSpeed = rampSpeedDynamic(currentSpeed, targetMagnitude, accelStep);

  // 3. APPLY PWM
  analogWrite(pwmPin, (int)currentSpeed);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  // Matrix Init
  botDisplay.begin();
  botDisplay.setIntensity(5); // Lower this if it flickers!
  randomSeed(analogRead(34)); 
  drawFace(eyeOpen);

  // Motor Pins
  pinMode(INA, OUTPUT); pinMode(INB, OUTPUT); pinMode(ENA, OUTPUT);
  pinMode(INC, OUTPUT); pinMode(IND, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(INA2, OUTPUT); pinMode(INB2, OUTPUT); pinMode(ENA2, OUTPUT);
  pinMode(INC2, OUTPUT); pinMode(IND2, OUTPUT); pinMode(ENB2, OUTPUT);

  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  unsigned long currentMillis = millis();

  // ==========================================
  // 1. MATRIX ANIMATION LOGIC 
  // ==========================================
  if (globalMode == 0) {
    if (currentMillis - lastModeSwitch >= modeDuration) {
      globalMode = 1;
      botDisplay.displayClear();
      botDisplay.displayText(zingers[random(numZingers)], PA_CENTER, 35, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    } else {
      if (blinkStep == 0 && currentMillis - lastLookTime >= nextLookDelay) {
        lastLookTime = currentMillis;
        int action = random(100);
        if (action < 15) currentEyeFrame = eyeLeft; 
        else if (action < 30) currentEyeFrame = eyeRight; 
        else if (action < 45) currentEyeFrame = eyeUp; 
        else if (action < 60) currentEyeFrame = eyeDown; 
        else currentEyeFrame = eyeOpen;
        drawFace(currentEyeFrame);
        nextLookDelay = random(600, 2500); 
      }
      if (blinkStep == 0 && currentMillis - lastBlinkTime >= nextBlinkDelay) {
        blinkStep = 1; lastBlinkTime = currentMillis; drawFace(eyeHalf); nextBlinkDelay = 40; 
      } else if (blinkStep == 1 && currentMillis - lastBlinkTime >= nextBlinkDelay) {
        blinkStep = 2; lastBlinkTime = currentMillis; drawFace(eyeClosed); nextBlinkDelay = random(40, 100); 
      } else if (blinkStep == 2 && currentMillis - lastBlinkTime >= nextBlinkDelay) {
        blinkStep = 3; lastBlinkTime = currentMillis; drawFace(eyeHalf); nextBlinkDelay = 40; 
      } else if (blinkStep == 3 && currentMillis - lastBlinkTime >= nextBlinkDelay) {
        blinkStep = 0; lastBlinkTime = currentMillis; drawFace(currentEyeFrame); nextBlinkDelay = random(2000, 6000); 
      }
    }
  } else if (globalMode == 1) {
    if (botDisplay.displayAnimate()) {
      globalMode = 0;
      lastModeSwitch = currentMillis;
      modeDuration = random(30000, 45000); 
      currentEyeFrame = eyeOpen; drawFace(eyeOpen);
      blinkStep = 0; lastBlinkTime = currentMillis; nextBlinkDelay = random(2000, 4000);
    }
  }
    // ==========================================
    // 2. RESTORED MOTOR CONTROL LOGIC 
    // ==========================================
    if (currentMillis - lastMotorUpdateTime >= 20) {
      lastMotorUpdateTime = currentMillis;
  
      int rawForward = -data.accelZ; 
      int rawRoll    = -data.accelX;  
      
      // --- 1. UNIFIED DEADZONE ---
      // Both axles must wake up at the exact same time to prevent dragging
      float deadzone = 4000; 
  
      // Dynamic Accel Step calculation
      float maxInput = max(abs((float)rawForward), abs((float)rawRoll)); 
      float tiltRatio = constrain((maxInput - deadzone) / (16000 - deadzone), 0.0, 1.0);
      float currentAccelStep = MIN_ACCEL + (tiltRatio * (MAX_ACCEL - MIN_ACCEL));
  
      // --- 2. INDEPENDENT POWER CURVES ---
      // Front uses smooth, gentle curves
      int forwardSpeedFront = applyExponentCurve(rawForward, deadzone, 16000, 160, 0.7);
      int rollSpeedFront    = applyExponentCurve(rawRoll, deadzone, 16000, 100, 0.5); 
  
      // Rear uses steeper curves (0.45 and 0.35) to push the heavy battery weight,
      // giving them an immediate surge of power as soon as the deadzone is crossed.
      int forwardSpeedRear = applyExponentCurve(rawForward, deadzone, 16000, 255, 0.10);
      int rollSpeedRear    = applyExponentCurve(rawRoll, deadzone, 16000, 150, 0.05);
  
      int strafeSpeedFront = 0, turnSpeedFront = 0;
      int strafeSpeedRear  = 0, turnSpeedRear  = 0;
  
      // THE BUTTON MODIFIER LOGIC
      if (data.triggerHeld == 1) {
        strafeSpeedFront = rollSpeedFront; 
        strafeSpeedRear  = rollSpeedRear;
        forwardSpeedFront = 0; 
        forwardSpeedRear  = 0;
      } else {
        turnSpeedFront = rollSpeedFront;   
        turnSpeedRear  = rollSpeedRear;
      }
      
      // Omni-Wheel Mixing 
      int targetFL = forwardSpeedFront + turnSpeedFront + strafeSpeedFront;
      int targetFR = forwardSpeedFront - turnSpeedFront - strafeSpeedFront;
      
      int targetBL = forwardSpeedRear + turnSpeedRear - strafeSpeedRear;
      int targetBR = forwardSpeedRear - turnSpeedRear + strafeSpeedRear;
  
      int maxTarget = max({abs(targetFL), abs(targetBL), abs(targetFR), abs(targetBR)});
      if (maxTarget > 255) {
        targetFL = (targetFL * 255) / maxTarget;
        targetBL = (targetBL * 255) / maxTarget;
        targetFR = (targetFR * 255) / maxTarget;
        targetBR = (targetBR * 255) / maxTarget;
      }
  
      // Drive Motors 
      driveMotor(targetFL, INA,  INB,  ENA,  curSpeedFL, currentAccelStep); 
      driveMotor(targetFR, INC,  IND,  ENB,  curSpeedFR, currentAccelStep); 
      driveMotor(targetBL, INA2, INB2, ENA2, curSpeedBL, currentAccelStep); 
      driveMotor(targetBR, INC2, IND2, ENB2, curSpeedBR, currentAccelStep); 
    }
}