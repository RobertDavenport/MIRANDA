#include <SoftwareSerial.h>

// Pinout for haptic unit control.
const uint8_t HAPTIC_MOTOR_1 = 0;
const uint8_t HAPTIC_MOTOR_2 = 1;

// Pinout for software serial
const uint8_t rx = 3;
const uint8_t tx = 4;

SoftwareSerial softSerial = SoftwareSerial(rx, tx);

void setup() {
  // Initialize the haptic motor's control pin.
  pinMode(HAPTIC_MOTOR_1, OUTPUT);
  pinMode(HAPTIC_MOTOR_2, OUTPUT);

  // Initialize software serial pins
  pinMode(rx, INPUT);
  pinMode(tx, OUTPUT); 
  
  Serial.begin(115200);

  softSerial.begin(115200);
}

void loop() {
  if(softSerial.available()){
    Serial.println(softSerial.read());
    //byte intensity = Serial.parseInt();
    //Serial.println(intensity);
    //feedback(intensity);
  }
 delay(150);
}

void feedback(int intensity){
  analogWrite(HAPTIC_MOTOR_1, intensity);
  analogWrite(HAPTIC_MOTOR_2, intensity);
  //Serial.println(intensity);
}
