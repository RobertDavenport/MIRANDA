#include <Wire.h> // for i2c communication
#include <TFLI2C.h> // TFLuna Library

TFLI2C tfli2c;

#define NEW_ADDRESS = 0x21;
#define OLD_ADDRESS = 0x10;

void setup() {
  Wire.begin();         // Init wire library

  //One Time Setup for changing TFLuna I2C Addresses.
  tfli2c.Set_I2C_Addr(NEW_ADDRESS, OLD_ADDRESS);
  tfli2c.Save_Settings(OLD_ADDRESS);
  tfli2c.Soft_Reset(OLD_ADDRESS);
}

void loop() {}
