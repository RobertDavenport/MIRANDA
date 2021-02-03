// Defines the min and max distances (range) for the 
// haptic feedback units to output (currently arbitrary numbers).
#define MAX_DISTANCE 10000
#define MIN_DISTANCE 0

// Value range of power that can be applied in PWM.
#define MIN_MOTOR_SPEED 0
#define MAX_MOTOR_SPEED 255

// Pinout for haptic unit control.
const int HAPTIC_MOTOR_1 = 5;

void setup() {
  // Initialize the haptic motor's control pin.
  pinMode(HAPTIC_MOTOR_1, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // Simulates a measured distance increasing every 250ms 
  // with the haptic unit.
  for(int i = MIN_DISTANCE; i < MAX_DISTANCE; i += 1000){
    feedback(HAPTIC_MOTOR_1,i);
    delay(250); 
  }
}

// Applies the proportional feedback of a passed distance
// to the haptic motor passed by its control pin.
void feedback(int hapticMotor, int dist) {
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
  dist = map(dist, MIN_DISTANCE, MAX_DISTANCE, MAX_MOTOR_SPEED, MIN_MOTOR_SPEED);
  Serial.println(dist);

  // PWM the haptic unit for the mapped intensity.
  analogWrite(hapticMotor, dist);
}
