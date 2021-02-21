#include <Wire.h> // for i2c communication
#include <TFLI2C.h> // TFLuna Library
#include<AUnit.h> // test framework

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
  // Uncomment to print distance's and intensities to the serial monitor; however,
  // test results will only print if this block is commented out
  /* tfli2c.getData(tfDistance, defaultAddress);
  Serial.print("Distance: ");
  Serial.println(tfDistance);
  hapticIntensityOne = feedbackProcess(tfDistance, maxDistance, maxFeedback);
  Serial.print("Intensity: ");
  Serial.println(hapticIntensityOne);
  delay(100);*/
  aunit::TestRunner::run();
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

// test cases for feedbackProcess
test(feedbackProcess) {
  assertEqual(feedbackProcess(0, maxDistance, maxFeedback), 255);
  assertEqual(feedbackProcess(50, maxDistance, maxFeedback), 227);
  assertEqual(feedbackProcess(100, maxDistance, maxFeedback), 198);
  assertEqual(feedbackProcess(150, maxDistance, maxFeedback), 170);
  assertEqual(feedbackProcess(200, maxDistance, maxFeedback), 142);
  assertEqual(feedbackProcess(250, maxDistance, maxFeedback), 113);
  assertEqual(feedbackProcess(300, maxDistance, maxFeedback), 85);
  assertEqual(feedbackProcess(350, maxDistance, maxFeedback), 57);
  assertEqual(feedbackProcess(400, maxDistance, maxFeedback), 28);
  assertEqual(feedbackProcess(450, maxDistance, maxFeedback), 0);
  assertEqual(feedbackProcess(500, maxDistance, maxFeedback), 0);
  assertEqual(feedbackProcess(600, maxDistance, maxFeedback), 0);
  assertEqual(feedbackProcess(700, maxDistance, maxFeedback), 0);
  assertEqual(feedbackProcess(800, maxDistance, maxFeedback), 0);
}
