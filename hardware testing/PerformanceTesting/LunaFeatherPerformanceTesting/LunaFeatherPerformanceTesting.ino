#include <Wire.h> // for i2c communication
#include <TFLI2C.h> // TFLuna Library

TFLI2C tfli2c;

// Defines the min and max distances (range) for the 
// haptic feedback units to output (currently arbitrary numbers).
#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

// Value range of power that can be applied in PWM.
#define MIN_MOTOR_SPEED 100
#define MAX_MOTOR_SPEED 255


int16_t tfDistance;   // cm
uint8_t defaultAddress = 0x10;
uint8_t tfSensorOne = 0x08;
unsigned char hapticIntensityOne;


// Pinout for haptic unit control.
const int HAPTIC_MOTOR_1 = 6;

// Pinout for sensor enable
const int TF_LUNA_1 = 10;


// Number of test cases per run
const int numTests = 1000;

// Number of runs
const int numRuns = 100;

unsigned long start, finished, elapsed;
unsigned long averageTime;
unsigned long runningSum;
unsigned long overallSum;
unsigned long overallAverage;

bool testRun;

void setup() {
  // Initialize the haptic motor's control pin.
  pinMode(HAPTIC_MOTOR_1, OUTPUT);
  pinMode(TF_LUNA_1, OUTPUT);
  Serial.begin(115200); // Init serial port with baud rate of 115200
  Wire.begin();         // Init wire library
  testRun = true;
}


void loop() {
  digitalWrite(TF_LUNA_1, LOW);
  delay(2000);
  if (testRun == true) {
    overallSum = 0;
    overallAverage = 0;
     for (int i = 1; i <= numRuns; i++) {
        Serial.print("Run #: ");
        Serial.println(i);
        averageTime = 0;
        runningSum = 0;
        for (int j = 1; j <= numTests; j++) {
          runningSum += sensorPerformance();
        }
        averageTime = runningSum / numTests;
        Serial.print("Average Time: ");
        Serial.println(averageTime);
        Serial.println();
        overallSum += averageTime;
        delay(50);
      }
      overallAverage = overallSum / numRuns;
      Serial.println("###############################################");
      Serial.print("Overall Average: ");
      Serial.print(overallAverage);
      Serial.println(" microseconds");
      testRun = false;
  }

}

unsigned long sensorPerformance () {
  start = micros();
  tfli2c.getData(tfDistance, defaultAddress);
  feedback(HAPTIC_MOTOR_1,tfDistance);
  finished = micros();
  elapsed = finished-start;
  return elapsed;
}

// Applies the proportional feedback of a passed distance
// to the haptic motor passed by its control pin.
void feedback(int hapticMotor, int dist) {
  // Sets the measured distance to MIN if dist < MIN or MAX if dist > MAX.
  if(dist == 0){
    analogWrite(hapticMotor, 0);
    return;
  }
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

  // PWM the haptic unit for the mapped intensity.
  analogWrite(hapticMotor, MIN_DISTANCE+dist);
}
