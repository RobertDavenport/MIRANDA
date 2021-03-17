#include <Arduino.h>
#include <TFLI2C.h>
#include <Wire.h>

// Min and max acceptable values for distance read in the from TF LUNA TOF sensor
#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

// Min and max motor speed for the distance-to-intensity mappings
#define MIN_MOTOR_SPEED 100
#define MAX_MOTOR_SPEED 255

int16_t tfDistance; // Holds the recorded distance
uint8_t tfAddress = 0x10; // Address of the TF LUNA sensor
uint8_t tfSensor1 = 0x08;
unsigned char hapticIntensityOne;

const int TF_LUNA_1 = 10; // Controls the power to the TF Luna sensor
TFLI2C tfli2c; 

void setup() {
  Serial.begin(9600);
  Wire.begin();
  digitalWrite(TF_LUNA_1, LOW); // Powers on the TF Luna
}

void loop() {
  pinMode(TF_LUNA_1, OUTPUT);
  tfli2c.getData(tfDistance, tfAddress);
  Serial.println(feedback(tfDistance));
  delay(150);
}

// Maps the read-in distance value to the range of possible distance values normalized to 
// the range of possible motor speeds.
int feedback(int dist){
  if(dist <= MIN_DISTANCE){return 0;} // Distance outside of min acceptable range.
  dist = min(MAX_DISTANCE, dist);
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MAX_MOTOR_SPEED, MIN_MOTOR_SPEED);
  return dist;
}
