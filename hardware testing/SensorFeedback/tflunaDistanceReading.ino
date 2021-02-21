#include <Wire.h> // for i2c communication
#include <TFLI2C.h> // TFLuna Library

TFLI2C tfli2c;

int16_t tfDistance;   // cm
uint8_t defaultAddress = 0x10;
uint8_t tfSensorOne = 0x08;
unsigned char hapticIntensityOne;
uint16_t maxDistance = 450; 
uint16_t maxFeedback = 255; 

void setup() {
  Serial.begin(115200); // Init serial port with baud rate of 115200
  sensorSetup(tfSensorOne, defaultAddress);
  Wire.begin();         // Init wire library
}


void loop() {
  tfli2c.getData(tfDistance, defaultAddress);
  Serial.print("Distance: ");
  Serial.println(tfDistance);
  hapticIntensityOne = feedbackProcess(tfDistance, maxDistance, maxFeedback);
  Serial.print("Intensity: ");
  Serial.println(hapticIntensityOne);
  delay(100);
}



// setup to change sensor address
void sensorSetup(uint8_t sensorAddress, uint8_t defaultAddress) {
   tfli2c.Set_I2C_Addr(sensorAddress, defaultAddress);
   tfli2c.Soft_Reset(sensorAddress);
}

// calculate the feedback intensity from the sensor's distance reading
unsigned char feedbackProcess (int16_t distance, int16_t maxDistance, int16_t maxFeedback) {
  unsigned char feedbackIntensity = 0;
  if (distance <= maxDistance) {
   feedbackIntensity = round(maxFeedback * (1.00 - float(distance)/450.0));
  }
  return feedbackIntensity;
}
