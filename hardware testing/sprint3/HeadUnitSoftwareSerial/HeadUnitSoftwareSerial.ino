#include <Arduino.h>
#include <TFLI2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

#define MIN_MOTOR_SPEED 100
#define MAX_MOTOR_SPEED 255

const byte rx = 11;
const byte tx = 12;

int16_t tfDistance;
uint8_t tfAddress = 0x10;
uint8_t tfSensor1 = 0x08;
unsigned char hapticIntensityOne;

const int TF_LUNA_1 = 10;
const int WRIST_0 = 9;

TFLI2C tfli2c;
SoftwareSerial softSerial = SoftwareSerial(rx, tx);



void setup() {
  pinMode(TF_LUNA_1, OUTPUT);
  pinMode(WRIST_0, OUTPUT);

  // software serial pins
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT);
  
  Serial.begin(115200);

  softSerial.begin(115200);
  
  Wire.begin();
  digitalWrite(TF_LUNA_1, LOW);
}

void loop() {
  tfli2c.getData(tfDistance, tfAddress);
  int feedBack = feedback(tfDistance);
  softSerial.println(feedBack);
  Serial.println(feedBack);
  delay(150);
}


int feedback(int dist){
  if(dist <= MIN_DISTANCE){return 0;}

  dist = min(MAX_DISTANCE, dist);
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MAX_MOTOR_SPEED, MIN_MOTOR_SPEED);
  return dist;
}
