#include <Wire.h> // for i2c communication
#include <TFLI2C.h> // TFLuna Library
#include<AUnit.h> // test framework

TFLI2C tfli2c;

// Defines the min and max distances (range) for the 
// haptic feedback units to output (currently arbitrary numbers).
#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

// Value range of power that can be applied in PWM.
#define MIN_MOTOR_SPEED 170
#define MAX_MOTOR_SPEED 255

// Boolean to run unit test
#define TEST_MODE  false


int16_t tfDistance;   // cm
uint8_t defaultAddress = 0x10;
uint8_t tfSensorOne = 0x08;
unsigned char hapticIntensityOne;
int distance;

// Pinout for haptic unit control.
const int HAPTIC_MOTOR_1 = 5;

void setup() {
  // Initialize the haptic motor's control pin.
  pinMode(HAPTIC_MOTOR_1, OUTPUT);
  Serial.begin(115200); // Init serial port with baud rate of 115200
  sensorSetup(tfSensorOne, defaultAddress);
  Wire.begin();         // Init wire library
  // Run Test  
}


void loop() {
  if (TEST_MODE == true)
  {
    aunit::TestRunner::run();
  }
  else
  {  
  tfli2c.getData(tfDistance, defaultAddress);
  Serial.print("Distance: ");
  Serial.println(tfDistance);
  //hapticIntensityOne = feedbackProcess(tfDistance, MAX_DISTANCE, MAX_MOTOR_SPEED);
  //Serial.print("Intensity: ");
  //Serial.println(hapticIntensityOne);
  //analogWrite(HAPTIC_MOTOR_1, hapticIntensityOne);
  distance = feedback(HAPTIC_MOTOR_1,tfDistance);
  // PWM the haptic unit for the mapped intensity.
  Serial.println(distance);
  analogWrite(HAPTIC_MOTOR_1, MAX_DISTANCE-distance);
  delay(200);
  }
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

// Applies the proportional feedback of a passed distance
// to the haptic motor passed by its control pin.
int feedback(int hapticMotor, int dist) {
  // Sets the measured distance to MIN if dist < MIN or MAX if dist > MAX.
  if(dist == 0){
    analogWrite(hapticMotor, 0);
    return dist;
  }
  dist = max(MIN_DISTANCE, dist);
  dist = min(MAX_DISTANCE, dist);
  
  // Maps the measured distance within the range of
  // accepted distances to the range of haptic 
  // feedback intensity.
  // Note: the mapping is reversed from distance (min, max) to 
  // motor speed (max, min) because the PWM pin is functioning
  // as the ground for the motor, which is powered from the 5V
  // rail.
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MAX_MOTOR_SPEED, MIN_MOTOR_SPEED);
  return dist;
}
