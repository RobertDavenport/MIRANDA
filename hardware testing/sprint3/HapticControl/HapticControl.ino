
// Pinout for haptic unit control.
const uint8_t HAPTIC_MOTOR_1 = 3;
const uint8_t HAPTIC_MOTOR_2 = 4;

void setup() {
  // Initialize the haptic motor's control pin.
  pinMode(HAPTIC_MOTOR_1, OUTPUT);
  pinMode(HAPTIC_MOTOR_2, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  feedback(analogRead(A6/2));
//  if(Serial.available()){
//    byte intensity = Serial.parseInt();
//    Serial.println(intensity);
//    feedback(intensity);
//  }
}

void feedback(int intensity){
  analogWrite(HAPTIC_MOTOR_1, intensity);
  analogWrite(HAPTIC_MOTOR_2, intensity);
  Serial.println(intensity);
}
