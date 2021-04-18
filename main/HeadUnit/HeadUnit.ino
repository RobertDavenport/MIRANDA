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
// temp mapping
char temp[4];

// Distance Array for sensor Readings
int16_t distanceReadings[3] = {0,0,0};

// Arrays for Accelerometer readings
double currentAccelerometer[3] = {0,0,0};
double initialAccelerometer[3] = {0,0,0};

// Arrays for Gyroscope readings
double currentGyroscope[3] = {0,0,0};
double initialGyroscope[3] = {0,0,0};


void setup() {
  Serial.begin(115200);
  Wire.begin();
  

  // Wake the MPU-6050/GY-521 sensor and take an initial reading.
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  // A delay seems to be needed for the initialization when the unit is powered on without code being uploaded
  delay(1000);
  initializeAccelGyro(initialAccelerometer, currentAccelerometer, initialGyroscope, currentGyroscope);
  
  // Setup Wifi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(my_ssid, password);
 
  delay(250); 
}

void loop() {
  readLidarArray(sensorArray, distanceReadings);
  printDistances(distanceReadings);
  readAccelGyro(initialAccelerometer, currentAccelerometer, initialGyroscope, currentGyroscope);
  printAccelGyroValues(initialAccelerometer, currentAccelerometer, initialGyroscope, currentGyroscope);
  tempMapping(distanceReadings, temp);
  // TODO: We will broadcast the final string after area mapping
  Serial.println(temp);
  udp.broadcastTo(temp, port);

  delay(150);
}

void tempMapping(int16_t *distanceReadings, char * temp){
  for(int i = 0; i<3; i++) {    
    if (distanceReadings[i] < 150) {
      temp[i] = 'h'; 
    }
    else if (distanceReadings[i] < 300) {
      temp[i] = 'g';      
    }
    else {
      temp[i] = 'f';        
    }
  }
}


// Gets distance readings for each sensor in the array and updates the array of distances
void readLidarArray(uint8_t *sensorArray, int16_t *distanceReadings) {
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

// Finds the current state of the Accelerometer and Gyroscope.
void readAccelGyro(double * initialAngles, double * currentAngles, double * initialGyroscope, double * currentGyroscope) {
  Wire.beginTransmission(MPU_addr); 
  Wire.write(0x3B); 
  Wire.endTransmission(false); 
  Wire.requestFrom(MPU_addr,6,true); 

  // Each value is stored in 2 1-byte registers in the sensor. Need to read both into 1 int16 for correct output.
  int16_t AccelX = Wire.read()<<8|Wire.read(); // reg: 0x3B & 0x3C
  int16_t AccelY = Wire.read()<<8|Wire.read(); // reg: 0x3D & 0x3E
  int16_t AccelZ = Wire.read()<<8|Wire.read(); // reg: 0x3F & 0x40

  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,6,true);

  int16_t GyroX = Wire.read()<<8|Wire.read(); // reg: 0x43 & 0x44
  int16_t GyroY = Wire.read()<<8|Wire.read(); // reg: 0x45 & 0x46
  int16_t GyroZ = Wire.read()<<8|Wire.read(); // reg: 0x47 & 0x48

  
  int16_t AccelAngX = map(AccelX,minVal,maxVal,-90,90); 
  int16_t AccelAngY = map(AccelY,minVal,maxVal,-90,90); 
  int16_t AccelAngZ = map(AccelZ,minVal,maxVal,-90,90);

  int16_t GyroAngX = map(GyroX,minVal,maxVal,-90,90); 
  int16_t GyroAngY = map(GyroY,minVal,maxVal,-90,90); 
  int16_t GyroAngZ = map(GyroZ,minVal,maxVal,-90,90);

  // Find the current rotation (x,y,z) and normalize against the initial rotation state.
  currentAccelerometer[0] = (RAD_TO_DEG * (atan2(-AccelAngY, -AccelAngZ)+PI)) - initialAccelerometer[0]; 
  currentAccelerometer[1] = (RAD_TO_DEG * (atan2(-AccelAngX, -AccelAngZ)+PI)) - initialAccelerometer[1]; 
  currentAccelerometer[2] = (RAD_TO_DEG * (atan2(-AccelAngY, -AccelAngX)+PI)) - initialAccelerometer[2];
  currentGyroscope[0] = (RAD_TO_DEG * (atan2(-GyroAngY, -GyroAngZ)+PI)) - initialGyroscope[0]; 
  currentGyroscope[1] = (RAD_TO_DEG * (atan2(-GyroAngX, -GyroAngZ)+PI)) - initialGyroscope[1]; 
  currentGyroscope[2] = (RAD_TO_DEG * (atan2(-GyroAngY, -GyroAngX)+PI)) - initialGyroscope[2];

}

// Initializes the gyroscope and takes the initial readings. 
void initializeAccelGyro(double * initialAngles, double * currentAngles, double * initialGyroscope, double * currentGyroscope) {

  // Take an initial reading and update initialAngles
  readAccelGyro(initialAccelerometer, currentAccelerometer, initialGyroscope, currentGyroscope);
  for (int i = 0; i < 3; i++){
    initialAccelerometer[i] = currentAccelerometer[i];
    initialGyroscope[i] = currentGyroscope[i];
  }
}

void printAccelGyroValues(double * initialAccelerometer, double * currentAccelerometer, double * initialGyroscope, double * currentGyroscope) {
  Serial.println("Initial Accelerometer: ");
  for (int i = 0; i < 3; i++){
    Serial.print(initialAccelerometer[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("Current Accelerometer: ");
  for (int i = 0; i < 3; i++){
    Serial.print(currentAccelerometer[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("Initial Gyroscope: ");
  for (int i = 0; i < 3; i++){
    Serial.print(initialGyroscope[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("Current Gyroscope: ");
  for (int i = 0; i < 3; i++){
    Serial.print(currentGyroscope[i]);
    Serial.print(" ");
  }
  Serial.println();
}
