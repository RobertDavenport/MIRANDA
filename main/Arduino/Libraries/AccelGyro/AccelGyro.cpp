#include <AccelGyro.h>
#include <Arduino.h>
#include <Wire.h>

// Default Constructor
AccelGyro::AccelGyro(){};

// Constructor with arguments of gyroscope x-y-z.
// Accelerometer to be implemeted
AccelGyro::AccelGyro(double *gx, double *gy, double *gz) {
	x = gx;
	y = gy;
	z = gz;
	
	// Initialize the MPU-6050 / GY-521 sensor.
	Wire.begin();
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x6B);
	Wire.write(0);
	
	// Initialize the angles from the starting position.
	initAngles();
}

// Finds the current state of the gyroscope.
void AccelGyro::computeAngles() {
	Wire.beginTransmission(MPU_addr); 
  	Wire.write(0x3B); 
  	Wire.endTransmission(false); 
  	Wire.requestFrom(MPU_addr,14,true); 
  
  	AcX=Wire.read()<<8|Wire.read(); // Each value is stored in 2 1-byte registers in the sensor;
  	AcY=Wire.read()<<8|Wire.read(); // need to read both into 1 int16 for correct output.
  	AcZ=Wire.read()<<8|Wire.read(); 
  
  	int xAng = map(AcX,minVal,maxVal,-90,90); 
  	int yAng = map(AcY,minVal,maxVal,-90,90); 
  	int zAng = map(AcZ,minVal,maxVal,-90,90);

	// Find the current rotation (x,y,z) and normalize against the initial rotation state.
  	*x = (RAD_TO_DEG * (atan2(-yAng, -zAng)+PI)) - ix; 
  	*y = (RAD_TO_DEG * (atan2(-xAng, -zAng)+PI)) - iy; 
  	*z = (RAD_TO_DEG * (atan2(-yAng, -xAng)+PI)) - iz;
}

// Initializes the angles of the gyroscope from the starting rotation.
void AccelGyro::initAngles() {
	Wire.beginTransmission(MPU_addr); 
  	Wire.write(0x3B); 
  	Wire.endTransmission(false); 
  	Wire.requestFrom(MPU_addr,14,true); 
  
  	AcX=Wire.read()<<8|Wire.read(); // Each value is stored in 2 1-byte registers in the sensor;
  	AcY=Wire.read()<<8|Wire.read(); // need to read both into 1 int16 for correct output.
  	AcZ=Wire.read()<<8|Wire.read(); 
  
  	int xAng = map(AcX,minVal,maxVal,-90,90); 
  	int yAng = map(AcY,minVal,maxVal,-90,90); 
  	int zAng = map(AcZ,minVal,maxVal,-90,90);
  	ix= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI); 
  	iy= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI); 
  	iz= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
}
