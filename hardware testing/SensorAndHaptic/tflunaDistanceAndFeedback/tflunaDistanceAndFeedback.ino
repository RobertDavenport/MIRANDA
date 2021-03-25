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


int16_t tfDistanceS1;
int16_t tfDistanceS2;// cm
uint8_t defaultAddress = 0x10;
uint8_t tfSensorOne = 0x20;
uint8_t tfSensorTwo = 0x21;
uint8_t tfSensorThree = 0x0A;
unsigned char hapticIntensityOne;


// Pinout for haptic unit control.
const int HAPTIC_MOTOR_1 = 6;

// Pinout for sensor enable
const int TF_LUNA_1 = 10;

void setup() {
  // Initialize the haptic motor's control pin.
  pinMode(HAPTIC_MOTOR_1, OUTPUT);
  pinMode(TF_LUNA_1, OUTPUT);
  Serial.begin(115200); // Init serial port with baud rate of 115200  
  //sensorSetup(tfSensorOne, defaultAddress);
  Wire.begin();         // Init wire library

  //One Time Setup for changing TFLuna I2C Addresses.
  //tfli2c.Set_I2C_Addr(tfSensorTwo, tfSensorOne);
  //tfli2c.Save_Settings(tfSensorOne);
  //tfli2c.Soft_Reset(tfSensorOne);
  //tfli2c.Set_I2C_Addr(tfSensorTwo, defaultAddress);
  //tfli2c.Save_Settings(defaultAddress);
  //tfli2c.Soft_Reset(defaultAddress);
  
}


void loop() {
  digitalWrite(TF_LUNA_1, LOW);
  tfli2c.getData(tfDistanceS2, tfSensorTwo);
  tfli2c.getData(tfDistanceS1, tfSensorOne);
  Serial.print("Distance SensorOne: ");
  Serial.println(tfDistanceS1);
  Serial.print("Distance SensorTwo: ");
  Serial.println(tfDistanceS2);
  feedback(HAPTIC_MOTOR_1,tfDistanceS1);
  delay(200);
}


// setup to change sensor address
void sensorSetup(uint8_t sensorAddress, uint8_t defaultAddress) {
   Serial.println("sensorAddress: " + String((char *)sensorAddress));
   Serial.println("defaultAddress: " + defaultAddress);
   tfli2c.Set_I2C_Addr(sensorAddress, defaultAddress);
   tfli2c.Save_Settings(defaultAddress);
   tfli2c.Soft_Reset(defaultAddress);
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
  Serial.print("Intensity: ");
  Serial.println(dist);

  // PWM the haptic unit for the mapped intensity.
  analogWrite(hapticMotor, MIN_DISTANCE+dist);
}
