// Defines the min and max distances (range) for the 
// haptic feedback units to output (currently arbitrary numbers).
#define MAX_DISTANCE 1024
#define MIN_DISTANCE 0

// Value range of power that can be applied in PWM.
#define MIN_MOTOR_SPEED 0
#define MAX_MOTOR_SPEED 255

// Pinout for haptic unit control.
const uint8_t HAPTIC_MOTOR_1 = 5;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if(Serial.available()){
    int motorSpeed = Serial.parseInt();
    int feedbackValue = feedback(motorSpeed);
    Serial.println(String(feedbackValue));
  }
}

int byteToInt(byte b){
  int n = 0;
  return(n &= b);
}

byte intToByte(int16_t i){
  return lowByte(i);
}

// Applies the proportional feedback of a passed distance
// to the haptic motor passed by its control pin.
int feedback(int16_t dist) {
  // Sets the measured distance to MIN if dist < MIN or MAX if dist > MAX.
  dist = max(MIN_DISTANCE, dist);
  dist = min(MAX_DISTANCE, dist);
  
  // Maps the measured distance within the range of
  // accepted distances to the range of haptic 
  // feedback intensity.
  // Note: the mapping is reversed from distance (min, max) to 
  // motor speed (max, min) because the PWM pin is functioning
  // as the ground for the motor, which is powered from the 5V
  // rail.
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);
  return dist;
}
