#include <Arduino.h>
#include <TFLI2C.h>
#include <Wire.h>

#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

#define MIN_MOTOR_SPEED 100
#define MAX_MOTOR_SPEED 255

int16_t tfDistance;
uint8_t tfAddress = 0x10;
uint8_t tfSensor1 = 0x08;
unsigned char hapticIntensityOne;

const int TF_LUNA_1 = 10;
TFLI2C tfli2c;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  digitalWrite(TF_LUNA_1, LOW);
}

void loop() {
  pinMode(TF_LUNA_1, OUTPUT);
  tfli2c.getData(tfDistance, tfAddress);
  Serial.println(feedback(tfDistance));
  delay(150);
}


int feedback(int dist){
  if(dist <= MIN_DISTANCE){return 0;}

  dist = min(MAX_DISTANCE, dist);
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MAX_MOTOR_SPEED, MIN_MOTOR_SPEED);
  return dist;
}
