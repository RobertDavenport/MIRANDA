#include <math.h>
#include <I2Cdev.h>
#include <TFLI2C.h>
#include <MPU6050.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <AsyncUDP.h>
#include <WiFi.h>

/* Determines if accelermeter will be used
   to account for movement when mapping the
   environment. */
#define MAP_AREA_WITHOUT_MOVEMENT

// Settings for area mapping structure
#define DPI 6
#define RADIAL_SEGMENTS 60
#define EMPTY -1

// Indices for sensor values
#define GYRO_X 0
#define GYRO_Y 1
#define GYRO_Z 2
#define ACCL_X 0
#define ACCL_Y 1
#define ACCL_Z 2

// Constants
#define ACCL_X_OFFSET 1.02
#define ACCL_Y_OFFSET 0.0005
#define ACCL_Z_OFFSET 0.155
#define NUM_LIDAR 3
#define NUM_HAPTICS 5
#define HAPTIC_CONTROL_LEVELS 8

/* Number of degrees in a cone that each haptic
   motor will be mapped to. */
#define DPH 15
#define MAX_DIST 100.0
#define MIN_DIST 0.0
#define MAX_READABLE_DIST 450.0
#define MIN_READABLE_DIST 0.0

// Math macros and definitions
#define PI 3.14159265
#define radians(d) (d*0.0174533)
#define degrees(r) (r*57.2958)
#define nearestK(n,k) (int)round(n/(float)k)
#define normalize(n) (((float)n-MIN_READABLE_DIST)/(MAX_READABLE_DIST - MIN_READABLE_DIST))*(MAX_DIST-MIN_DIST)+MIN_DIST;

// Data structures for area mapping and haptic control.
struct Sensors {
  float lidar[NUM_LIDAR];
  float accel[3];
  float gyro[3] = {0,0,0};
  bool lock = false;
};
struct AreaMap {
  float angles[RADIAL_SEGMENTS];
  float distances[RADIAL_SEGMENTS];
  bool lock = false;
};
struct HapticCommand {
  char hapticsArr[NUM_HAPTICS+2] = "hhhhh\0";
  char *haptics = hapticsArr;
  bool lock;
};

const uint8_t lidarAddresses[3] = {0x10, 0x20, 0x21};
const float lidarAngles[NUM_LIDAR] = {-15, 0, 15};
const float LIDAR_WIDTH = 30;
const float gyroOffset[3] = {0.0035025, 0.0025, 0.0019};
const float hapticOutputAngles[NUM_HAPTICS] = {-30, -15, 0, 15, 30};

float gyro[3];

// Definitions for haptic control intensity-to-distance mappings
const char HAPTIC_MAPPINGS_CHAR[HAPTIC_CONTROL_LEVELS] = {'a','b','c','d','e','f','g','h'};
const float HAPTIC_MAPPINGS_DIST[HAPTIC_CONTROL_LEVELS][2] = {{0,10},
                                                        {10,15},
                                                        {15,20},
                                                        {20,25},
                                                        {25,30},
                                                        {35,40},
                                                        {40,45},
                                                        {45,MAX_DIST+1}};

struct AreaMap areaMap; // Perm. struct for storing mapped area.
struct AreaMap tempMap; // Temp. struct for updating mapped area.
struct Sensors sensors; // Stores the rt-database of sensor values.
struct HapticCommand command; // Stores most recent haptic command.

// Wifi Connection Setup
const char *my_ssid = "esp32_ssid";
const char *password = "password";
int PORT = 1234;

unsigned long prevTime = 0;

Adafruit_MPU6050 mpu;
TFLI2C tfli2c;
AsyncUDP udp;

// Core-assigned task handles
TaskHandle_t AreaMappingTask;
TaskHandle_t ControlTask;

void setup() {
  Serial.begin(115200);  
  Wire.begin();
  mpu.begin();
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Setup Wifi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(my_ssid, password);

  // Control (networking and sensor reading) performed on core 0
  xTaskCreatePinnedToCore(
              control,        // Function
              "Control",      // Name
              16384,          // Stack size
              NULL,           // Parameters
              2,              // Priority
              &ControlTask,   // TaskHandle
              0);             // Core No.

  
  // Area mapping performed on core 1
  
  xTaskCreatePinnedToCore(
              areaMapping,      // Function
              "AreaMapping",    // Name
              16384,            // Stack size
              NULL,             // Parameters
              2,                // Priority
              &AreaMappingTask, // TaskHandle
              1);               // Core No.
  
  prevTime = millis();
  delay(1000);
}

void loop() {
}

void control( void * params ) {
  while(true) {

    /* Lock the sensor struct for copying */
    while(sensors.lock) { delay(1); }
    sensors.lock = true;
    Sensors temp = sensors;
    sensors.lock = false;

    /* Update the sensor struct */
    temp = readSensors(temp);

    /* Lock and replace the sensor struct */
    while(sensors.lock) { delay(1); }
    sensors.lock = true;
    sensors = temp;
    sensors.lock = false;

    /* Lock the haptic command for copying */
    while(command.lock) { delay(1); }
    command.lock = true;
    HapticCommand cmd = command;
    command.lock = false;

    Serial.println(cmd.haptics);
    udpBroadcast(cmd);
    delay(125);
  }
}

// Area Mapping
void areaMapping( void * params ) {

  while(true) {
    /* Wait for sensors data to be unlocked
      for use by this thread. */
    while(sensors.lock) { delay(1); }

    /* Temporarily lock the sensor data
      for use by this thread. The data
      is copied into a temporary struct
      because this function takes a long
      time to execute. */
    sensors.lock = true;
    struct Sensors tempSensors;
    tempSensors = sensors;
    sensors.lock = false;

    //tempSensors = normalizeDistances(tempSensors);
    updateMapDistances(tempSensors);
    updateHapticCommand(tempSensors);
    delay(300);
  }
}

/* ~~~~~~~ Misc functions ~~~~~~~ */
float getDeltaTime() {
  unsigned long nt = millis();
  float dt = (nt - prevTime) * 0.001;
  prevTime = nt;
  return dt;
}

/* ~~~~~~~ Area mapping functions ~~~~~~~ */
inline void angleMagitudeToComponent( float angle, float magnitude, float * coords ) {
  coords[0] = cosf(radians(angle)) * magnitude;
  coords[1] = sinf(radians(angle)) * magnitude;
}

inline void componentToAngleMagnitude( float x, float z, float * vec ) {
  vec[0] = degrees(atan2f(z,x));
  vec[1] = sqrtf((x * x)+(z * z));
  if(vec[0] < 0) { vec[0] += 360; }
}

// Update Mapping Distances
void updateMapDistances(Sensors s) {
  for(int i = 0; i < NUM_LIDAR; i++) {
    float angle = degrees(sensors.gyro[GYRO_Y]) + lidarAngles[i];
    if(angle > 360) {
      angle = angle - 360;
    }
    if(angle < 0) {
      angle = 360 + angle;
    }
    int index = nearestK(angle, DPI) % RADIAL_SEGMENTS;
    areaMap.angles[index] = fmodf(angle, 360);
    areaMap.distances[index] = s.lidar[i];
  }
}

// Normalize Distance data
Sensors normalizeDistances(Sensors s) {
  float relativeAngle = cosf(radians(s.gyro[GYRO_X]));
  for(int i = 0; i < NUM_LIDAR; i++) {
    s.lidar[i] = s.lidar[i] * relativeAngle;
  }
  return s;
}

#ifndef MAP_AREA_WITHOUT_MOVEMENT
// Update Area Mapping
void updateMapLocation(Sensors s) {
  
  // Return if there was no movement to update.
  if(s.accel[ACCL_X] == 0 && s.accel[ACCL_Z] == 0) { return; }

  for(int i = 0; i < RADIAL_SEGMENTS; i ++) {
    tempMap.angles[i] = i*DPI;
    tempMap.distances[i] = EMPTY;
  }

  for(int i = 0; i < RADIAL_SEGMENTS; i ++) {
    if(areaMap.distances[i] <= 0) { continue; }

    float angle = fmodf(areaMap.angles[i] + 180, 360);
    float dist = areaMap.distances[i];

    float componentVector[2];
    angleMagitudeToComponent(angle, dist, componentVector);
    componentVector[0] += s.accel[ACCL_Z];
    componentVector[0] += s.accel[ACCL_X];

    float amVector[2];
    componentToAngleMagnitude(componentVector[0], componentVector[2],
                              amVector);
    
    int index = ((int)round(amVector[0] / DPI)) % RADIAL_SEGMENTS;
    if(tempMap.distances[i] == EMPTY || (tempMap.distances[i] > amVector[1] && tempMap.distances[i] != EMPTY)) {
      tempMap.angles[index] = amVector[0];
      tempMap.distances[index] = amVector[1];
    }
  }

  for(int i = 0; i < RADIAL_SEGMENTS; i ++) {
    areaMap.angles[i] = tempMap.angles[i];
    areaMap.distances[i] = tempMap.distances[i];
  } 
  
  // Swap tempMap and areaMap w/ pointers, needs testing.
  /*
  struct AreaMap *temp;
  temp = &areaMap;
  areaMap = tempMap;
  tempMap = temp;
  */
}

// Normalize Accelermeter
Sensors normalizeAccelermeter(Sensors s) {
  float acclVector[2];
  componentToAngleMagnitude(s.accel[ACCL_X], s.accel[ACCL_Z], acclVector);
  acclVector[0] = fmodf(acclVector[0] + s.gyro[GYRO_X], 360);
  float acclComps[2];
  angleMagitudeToComponent(acclVector[0], acclVector[1], acclComps);
  s.accel[ACCL_X] = acclComps[0];
  s.accel[ACCL_Z] = acclComps[1];
  return s;
}
#endif

// Update Haptic Data
void updateHapticCommand(Sensors s) {
    char newCommand[NUM_HAPTICS];
    for(int i = 0; i < NUM_HAPTICS; i++)
      newCommand[i] = HAPTIC_MAPPINGS_CHAR[HAPTIC_CONTROL_LEVELS-1];
    for(int i = 0; i < NUM_HAPTICS; i++) {
      float angle = degrees(sensors.gyro[GYRO_Y]) + hapticOutputAngles[i];
      if(angle > 360) {
        angle = angle - 360;
      }
      if(angle < 0) {
        angle = 360 + angle;
      }
      float dist = areaDistanceQuery(angle, DPH);
      char mapping = HAPTIC_MAPPINGS_CHAR[HAPTIC_CONTROL_LEVELS-1];
      for(int j = 0; j < HAPTIC_CONTROL_LEVELS; j++){
        if(dist >= HAPTIC_MAPPINGS_DIST[j][0] && dist < HAPTIC_MAPPINGS_DIST[j][1]) {
          mapping = HAPTIC_MAPPINGS_CHAR[j];
          break;
        }
      }
      newCommand[i] = mapping;
    }
    /* Wait for command data to be unlocked
      for use by this thread. */
    while(command.lock) { delay(1); }
    command.lock = true;
    for(int i = 0; i < NUM_HAPTICS; i++)
      command.hapticsArr[i] = newCommand[i];
    command.lock = false;
}

// Get Area Distance
float areaDistanceQuery(float angle, float coneWidth) {
  float distance = MAX_DIST + 1;
  int indexStart = angle/DPI - (coneWidth/2)/DPI;
  int indexEnd = angle/DPI + (coneWidth/2)/DPI;
  for(; indexStart <= indexEnd; indexStart++){
    int index = indexStart % RADIAL_SEGMENTS;
    if(index < 0) { index = RADIAL_SEGMENTS + index; }
    if(areaMap.distances[index] <= 0) { continue; }
    if(distance > areaMap.distances[index]) { distance = areaMap.distances[index]; }
  }
  if(distance == MAX_DIST + 1) { return 0; }
  return distance;
}

/* ~~~~~~~ Sensor reading functions ~~~~~~~ */
void getGyroAngles(float * gyro, sensors_event_t gevent, float deltaTime) {
  auto updateAngle = [&](float ang, float offset, float upd, float dt) -> float {
    ang = ang + offset + upd * dt;
    if(ang > 2*PI)
      ang = ang - 2*PI;
    if(ang < 0)
      ang = 2*PI - ang;
    return ang;
  };
  gyro[2] = updateAngle(gyro[2], gyroOffset[2], gevent.gyro.x, deltaTime);
  gyro[1] = updateAngle(gyro[1], gyroOffset[1], gevent.gyro.y, deltaTime);
  gyro[0] = updateAngle(gyro[0], gyroOffset[0], gevent.gyro.z, deltaTime);
}

/* ~~~~~~~ Control functions ~~~~~~~ */
Sensors readSensors(Sensors old) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float dt = getDeltaTime();
  getGyroAngles(old.gyro, g, dt);
  readLidarArray(old.lidar);
  return old;
}

// Read from Lidar Array
void readLidarArray(float * lidar) {
  for(int i = 0; i<3; i++){
    int16_t tempDistance;
    tfli2c.getData(tempDistance, lidarAddresses[i]);
    //Serial.println(tempDistance);
    lidar[i] = normalize(tempDistance);
  }
}

// Broadcast haptic data via UDP
void udpBroadcast(HapticCommand cmd) {
  //for(int i = 0; i < NUM_HAPTICS; i++)
  //  Serial.print(cmd.haptics[i]);
  //Serial.println();
  udp.broadcastTo(cmd.haptics,PORT);
}
