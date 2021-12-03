/* 
 *  How to use the MPU6050 accelerometer and gyroscope with Arduino tutorial
 *  By Gus Baker, roboticsexplained.com
 */

#include <Wire.h>

const int MPU_ADDRESS = 0x68;             // Address of the MPU6050 on the I2C line

// The address of the internal registers:
const int RESET = 0x6B;                   // Power Management 1
const int GYRO_CONFIG = 0x1B;             // Gyroscope configuration
const int ACCEL_CONFIG = 0x1C;            // Accelerometer configuration
const int ACCEL_OUT = 0x3B;               // First byte of 6 accelerometer output bytes
const int TEMP_OUT = 0x41;                // First byte of the 2 temperature output bytes
const int GYRO_OUT = 0x43;                // First byte of the 6 gyroscope output bytes

float AccelX, AccelY, AccelZ;
float GyroX, GyroY, GyroZ;
float mpuTemp;


void setup() {
  Serial.begin(9600);
  Wire.begin();                             // Initiate the Wire library and join the I2C bus as a master
 
  Wire.beginTransmission(MPU_ADDRESS);      // Start communication with the device on address 0x68 (the MPU6050)
  Wire.write(RESET);                        // Talk to register 0x6B
  Wire.write(0x01);                         // Place a 1 in the 6B register to make a reset
  Wire.endTransmission();                   // End transmission with the MPU6050

  // Set the full-scale range of the gyroscope to +- 500 deg/s (default = +-250 deg/s)
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(GYRO_CONFIG);
  Wire.write(0x8);
  Wire.endTransmission();

  // Set the full-scale range of the accelerometer to +-8g (default = +- 2g)
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(ACCEL_CONFIG);
  Wire.write(0x10);
  Wire.endTransmission();
}

void loop() {
  // Get temperature data
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(TEMP_OUT);
  Wire.endTransmission();

  Wire.requestFrom(MPU_ADDRESS, 2); 
  mpuTemp = (Wire.read() << 8 | Wire.read())/340.0 + 36.53;


  // Get accelerometer data
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(ACCEL_OUT);
  Wire.endTransmission();

  Wire.requestFrom(MPU_ADDRESS, 6); 
  AccelX = (Wire.read() << 8 | Wire.read())/4096.0;
  AccelY = (Wire.read() << 8 | Wire.read())/4096.0;
  AccelZ = (Wire.read() << 8 | Wire.read())/4096.0;


  // Get gyroscope data
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(GYRO_OUT);
  Wire.endTransmission();

  Wire.requestFrom(MPU_ADDRESS, 6); 
  GyroX = (Wire.read() << 8 | Wire.read())/65.5;
  GyroY = (Wire.read() << 8 | Wire.read())/65.5;
  GyroZ = (Wire.read() << 8 | Wire.read())/65.5;


  // Print all data
  Serial.print("Temp = ");
  Serial.print(mpuTemp, 2);
  Serial.println( "degC");

  Serial.print("AccelX = ");
  Serial.print(AccelX);
  Serial.print( "g, ");  
  Serial.print("AccelY = ");
  Serial.print(AccelY);
  Serial.print( "g, "); 
  Serial.print("AccelZ = ");
  Serial.print(AccelZ);
  Serial.println( "g"); 

  Serial.print("GyroX = ");
  Serial.print(GyroX);
  Serial.print( "deg/s, ");  
  Serial.print("GyroY = ");
  Serial.print(GyroY);
  Serial.print( "deg/s, "); 
  Serial.print("GyroZ = ");
  Serial.print(GyroZ);
  Serial.println( "deg/s"); 
  
  Serial.println("");
  delay(1000);
}