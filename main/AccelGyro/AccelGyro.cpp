#include <AccelGyro.h>
#include <Arduino.h>
#include <Wire.h>

AccelGyro::AccelGyro(){};

AccelGyro::AccelGyro(double *agx, double *agy, double *agz) {
	x = agx;
	y = agy;
	z = agz;

	Wire.begin();
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x6B);
	Wire.write(0);
	initAngles();
}

void AccelGyro::computeAngles() {
	Wire.beginTransmission(MPU_addr); 
  	Wire.write(0x3B); 
  	Wire.endTransmission(false); 
  	Wire.requestFrom(MPU_addr,14,true); 
  
  	AcX=Wire.read()<<8|Wire.read(); 
  	AcY=Wire.read()<<8|Wire.read(); 
  	AcZ=Wire.read()<<8|Wire.read(); 
  
  	int xAng = map(AcX,minVal,maxVal,-90,90); 
  	int yAng = map(AcY,minVal,maxVal,-90,90); 
  	int zAng = map(AcZ,minVal,maxVal,-90,90);

  	*x = (RAD_TO_DEG * (atan2(-yAng, -zAng)+PI)) - ix; 
  	*y = (RAD_TO_DEG * (atan2(-xAng, -zAng)+PI)) - iy; 
  	*z = (RAD_TO_DEG * (atan2(-yAng, -xAng)+PI)) - iz;
}

void AccelGyro::initAngles() {
	Wire.beginTransmission(MPU_addr); 
  	Wire.write(0x3B); 
  	Wire.endTransmission(false); 
  	Wire.requestFrom(MPU_addr,14,true); 
  
  	AcX=Wire.read()<<8|Wire.read(); 
  	AcY=Wire.read()<<8|Wire.read(); 
  	AcZ=Wire.read()<<8|Wire.read(); 
  
  	int xAng = map(AcX,minVal,maxVal,-90,90); 
  	int yAng = map(AcY,minVal,maxVal,-90,90); 
  	int zAng = map(AcZ,minVal,maxVal,-90,90);
  	ix= RAD_TO_DEG * (atan2(-yAng, -zAng)+PI); 
  	iy= RAD_TO_DEG * (atan2(-xAng, -zAng)+PI); 
  	iz= RAD_TO_DEG * (atan2(-yAng, -xAng)+PI);
}
