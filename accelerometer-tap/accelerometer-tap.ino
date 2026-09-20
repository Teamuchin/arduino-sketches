// Pins defined from your Oz+ source file
#include "SparkFunLSM6DS3.h" // Include the library [cite: 5]
#include "Wire.h"            // Needed for I2C communication [cite: 5]
#define RGB_RED 9
#define RGB_GREEN 10
#define RGB_BLUE 11

// Create the sensor object (I2C mode, Address 0x6A) 
LSM6DS3 myIMU(I2C_MODE, 0x6A);
// Global variables to store the "smoothed" values
float smoothX = 0, smoothY = 0, smoothZ = 0;
// Lower = smoother but slower. Try 0.05 to 0.1 for a "glow" effect.
float alpha = 0.05;

const int interruptPin = 7;    // [cite: 13]
volatile int tapCount = 0;     // 'volatile' is needed for variables used in interrupts

void setup() {
  // Initialize pins as outputs
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);

  Serial.begin(9600);
  //while (!Serial); // Wait for Serial Monitor to open
  Serial.println("Oz+ Serial Connected!");
  myIMU.begin();

  myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x0E);   // Enable X, Y, Z tap
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_THS_6D, 0x04); // Set threshold
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_INT_DUR2, 0x7F);   // Set timing
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_MD1_CFG, 0x48);    // Route to INT1 pin

  pinMode(interruptPin, INPUT); // [cite: 170]
  
  // Listen for the pulse on Pin 7 [cite: 151]
  attachInterrupt(digitalPinToInterrupt(interruptPin), tapISR, RISING);
}
void tapISR() {
  tapCount++; // Just record that a tap happened 
}

void loop() {
  
  // 1. Read the raw data
  float rawX = myIMU.readFloatAccelX();
  float rawY = myIMU.readFloatAccelY();
  float rawZ = myIMU.readFloatAccelZ();

  // 2. Apply the smoothing formula
  smoothX = (rawX * alpha) + (smoothX * (1.0 - alpha));
  smoothY = (rawY * alpha) + (smoothY * (1.0 - alpha));
  smoothZ = (rawZ * alpha) + (smoothZ * (1.0 - alpha));

  // 3. Map to 0-255 and constrain
  // We use constrain because (x+1)*128 can occasionally hit 256
  int redVal   = constrain((smoothX + 1) * 128, 0, 255);
  int greenVal = constrain((smoothY + 1) * 128, 0, 255);
  int blueVal  = constrain((smoothZ + 1) * 128, 0, 255);

  // 4. Update the LEDs [cite: 13, 80]
  analogWrite(9,  redVal);   // RGB_RED
  analogWrite(10, greenVal); // RGB_GREEN
  analogWrite(11, blueVal);  // RGB_BLUE

  // Small delay to let the eye actually see the change
  delay(10);
  /*
  if (tapCount > 0) {
    Serial.println("OZ+ TAPPED!");
    
    // Reset the count so we can wait for the next one [cite: 227]
    tapCount = 0; 
  }*/
}

