#include <Wire.h> // for i2c communication
#include <TFLI2C.h> // TFLuna Library
#include<AUnit.h> // test framework

TFLI2C tfli2c;

// Defines the min and max distances (range) for the 
// haptic feedback units to output (currently arbitrary numbers).
#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

#define MIN_MOTOR_SPEED 170
#define MAX_MOTOR_SPEED 255

// Boolean to run unit test
#define TEST_MODE  true


int16_t tfDistance;   // cm
uint8_t defaultAddress = 0x10;
uint8_t tfSensorOne = 0x08;
unsigned char hapticIntensityOne;

int distance;

// Pinout for haptic unit control.
const int HAPTIC_MOTOR_1 = 5;
void setup() {
  Serial.begin(115200); // Init serial port with baud rate of 115200
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


//unsigned char feedbackProcess (int16_t distance, int16_t maxDistance, int16_t ma);

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
  

  // as the ground for the motor, which is powered from the 5V
  // rail.
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MAX_MOTOR_SPEED, MIN_MOTOR_SPEED);
  Serial.println(dist);

  // PWM the haptic unit for the mapped intensity.
  analogWrite(hapticMotor, MAX_DISTANCE-dist);
  return dist;
}
