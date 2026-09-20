#include "SparkFunLSM6DS3.h"
#include "Wire.h"

LSM6DS3 myIMU(I2C_MODE, 0x6A); 

void setup() {
  Serial.begin(9600);
  while (!Serial); 
  Serial.println("System Online. Starting Sensor...");

  if (myIMU.begin() != 0) {
    Serial.println("Sensor Error!");
    while(1); 
  }

  // 1. Wake up the Accelerometer (Crucial!)
  // Set Output Data Rate to 416Hz as seen in your source 
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x60); 

  // 2. Enable Tap on X, Y, Z [cite: 163]
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x0E);   

  // 3. Set Threshold to 0x01 (Extremely sensitive) [cite: 164]
  // In your source this was 0x04. 0x01 will trigger at the slightest touch.
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_THS_6D, 0x0A); 

  // 4. Set Timing Windows [cite: 165, 166]
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_INT_DUR2, 0xAF);   
  myIMU.writeRegister(LSM6DS3_ACC_GYRO_WAKE_UP_THS, 0x80); 

  Serial.println("Ready! Try flicking the board with your finger.");
}

void loop() {
  uint8_t tapSource;
  // Read the status register directly
  myIMU.readRegister(&tapSource, LSM6DS3_ACC_GYRO_TAP_SRC);

  // Bit 6 (0x40) is Single Tap, Bit 5 (0x20) is Double Tap
  if (tapSource) { 
    Serial.println("TAP");
    delay(1000);
  } 
}