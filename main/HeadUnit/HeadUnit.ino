#include <Arduino.h>
#include <TFLI2C.h>
#include <Wire.h>
#include <WiFi.h>
#include <AsyncUDP.h>

TFLI2C tfli2c;

// Wifi Connection Setup
const char *my_ssid = "esp32_ssid";
const char *password = "password";
int port = 1234;

AsyncUDP udp;

// Min and max acceptable values for distance read in the from TF LUNA TOF sensor
#define MAX_DISTANCE 450
#define MIN_DISTANCE 1

// Min and max motor speed for the distance-to-intensity mappings
#define MIN_MOTOR_SPEED 100
#define MAX_MOTOR_SPEED 255

// Define variables for accelerometer
#define MPU_addr 0x68
#define minVal 265
#define maxVal 402

// Sensor Addresses
uint8_t TFSENSOR_LEFT = 0x10;
uint8_t TFSENSOR_MIDDLE = 0x20;
uint8_t TFSENSOR_RIGHT = 0x21;
// Sensor Array
uint8_t sensorArray[3] = {TFSENSOR_LEFT, TFSENSOR_MIDDLE, TFSENSOR_RIGHT};

// Distance Array for sensor Readings
int16_t distanceReadings[3] = {0,0,0};

// Angle Arrays for Accelerometer readings
double currentAngles[3] = {0,0,0};
double initialAngles[3] = {0,0,0};



//unsigned char hapticIntensityOne;

//const int TF_LUNA_1 = 10; // Controls the power to the TF Luna sensor



void setup() {
  //pinMode(TF_LUNA_PINOUT, OUTPUT);
  Serial.begin(115200);
  Wire.begin();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(my_ssid, password);

  //digitalWrite(TF_LUNA_1, LOW); // Powers on the TF Luna

  // Initialize the MPU-6050 / GY-521 sensor and take an initial reading.
  initializeSensor(initialAngles, currentAngles);
  delay(250); 
}

void loop() {
  readFromArray(sensorArray, distanceReadings);
  printDistances(distanceReadings);
  computeAngles(initialAngles, currentAngles);
  printValues(initialAngles, currentAngles);
  // TODO: We will broadcast the final string after area mapping
  udp.broadcastTo("ABCDEFGH", port);
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

// Finds the current state of the gyroscope.
void computeAngles(double * initialAngles, double * currentAngles) {
  Wire.beginTransmission(MPU_addr); 
  Wire.write(0x3B); 
  Wire.endTransmission(false); 
  Wire.requestFrom(MPU_addr,14,true); 
  
  int16_t AcX = Wire.read()<<8|Wire.read(); // Each value is stored in 2 1-byte registers in the sensor;
  int16_t AcY = Wire.read()<<8|Wire.read(); // need to read both into 1 int16 for correct output.
  int16_t AcZ = Wire.read()<<8|Wire.read(); 
  
  int16_t xAng = map(AcX,minVal,maxVal,-90,90); 
  int16_t yAng = map(AcY,minVal,maxVal,-90,90); 
  int16_t zAng = map(AcZ,minVal,maxVal,-90,90);

  // Find the current rotation (x,y,z) and normalize against the initial rotation state.
  currentAngles[0] = (RAD_TO_DEG * (atan2(-yAng, -zAng)+PI)) - initialAngles[0]; 
  currentAngles[1] = (RAD_TO_DEG * (atan2(-xAng, -zAng)+PI)) - initialAngles[1]; 
  currentAngles[2] = (RAD_TO_DEG * (atan2(-yAng, -xAng)+PI)) - initialAngles[2];

}

// Initializes the gyroscope and takes the initial readings. 
void initializeSensor(double * initialAngles, double * currentAngles) {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Take an initial reading and update initialAngles
  computeAngles(initialAngles, currentAngles);
  for (int i = 0; i < 3; i++){
    initialAngles[i] = currentAngles[i];
  }
}

void printValues(double * initialAngles, double * currentAngles) {
  for (int i = 0; i < 3; i++){
    Serial.print(initialAngles[i]);
    Serial.print(" ");
  }
  Serial.println();
  for (int i = 0; i < 3; i++){
    Serial.print(currentAngles[i]);
    Serial.print(" ");
  }
  Serial.println();
}
