#include <Arduino.h>
#include <TFLI2C.h>
#include <Wire.h>

TFLI2C tfli2c;

// Min and max acceptable values for distance read in the from TF LUNA TOF sensor
#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

// Min and max motor speed for the distance-to-intensity mappings
#define MIN_MOTOR_SPEED 100
#define MAX_MOTOR_SPEED 255

// Sensor Addresses
uint8_t TFSENSOR_LEFT = 0x10;
uint8_t TFSENSOR_MIDDLE = 0x20;
uint8_t TFSENSOR_RIGHT = 0x21;
// Sensor Array
uint8_t sensorArray[3] = {TFSENSOR_LEFT, TFSENSOR_MIDDLE, TFSENSOR_RIGHT};

// Distance Array for sensor Readings
int16_t distanceReadings[3] = {0,0,0};

unsigned char hapticIntensityOne;

const int TF_LUNA_1 = 10; // Controls the power to the TF Luna sensor



void setup() {
  //pinMode(TF_LUNA_PINOUT, OUTPUT);
  Serial.begin(115200);
  Wire.begin();
  //digitalWrite(TF_LUNA_1, LOW); // Powers on the TF Luna
}

void loop() {
  readFromArray(sensorArray, distanceReadings);
  printDistances(distanceReadings);
  delay(150);
}




// Gets distance readings for each sensor in the array and updates the array of distances
void readFromArray(uint8_t *sensorArray, int16_t *distanceReadings) {
  for(int i = 0; i<3; i++){
    tfli2c.getData(distanceReadings[i], sensorArray[i]);
  }
}

// prints Distance readings to serial
void printDistances(int16_t *distanceReadings) {
  Serial.print("Left: ");
  Serial.println(distanceReadings[0]);
  Serial.print("Middle: ");
  Serial.println(distanceReadings[1]);
  Serial.print("Right: ");
  Serial.println(distanceReadings[2]);
}

// Maps the read-in distance value to the range of possible distance values normalized to 
// the range of possible motor speeds.
int feedback(int dist){
  if(dist <= MIN_DISTANCE){return 0;} // Distance outside of min acceptable range.
  dist = min(MAX_DISTANCE, dist);
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MAX_MOTOR_SPEED, MIN_MOTOR_SPEED);
  return dist;
}
