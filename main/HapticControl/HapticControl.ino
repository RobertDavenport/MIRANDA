
// Pinout for haptic unit control.
const uint8_t HAPTIC_MOTOR_1 = 3;
const uint8_t HAPTIC_MOTOR_2 = 4;

void setup() {
  // Initialize the haptic motors' control pins.
  pinMode(HAPTIC_MOTOR_1, OUTPUT);
  pinMode(HAPTIC_MOTOR_2, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // Checks if data is in the Serial buffer
  if(Serial.available()){
    byte intensity = Serial.parseInt();
    Serial.println(intensity); // Echos the input
    feedback(intensity); // Update the haptic intensity.
  }
}

// Writes the passesd intensity to the haptic motors by PWM.
void feedback(int intensity){
  analogWrite(HAPTIC_MOTOR_1, intensity);
  analogWrite(HAPTIC_MOTOR_2, intensity);
}
