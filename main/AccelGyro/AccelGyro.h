#ifndef AccelGyro_H
#define AccelGyro_H

#include <Arduino.h>
#include <Wire.h>

#define MPU_addr 0x68
#define minVal 265
#define maxVal 402

class AccelGyro
{
	public:
		AccelGyro();
		AccelGyro(double *agx, double *agy, double *agz);
		void computeAngles();
		void initAngles();
	private:
		int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
		double *x, *y, *z, ix, iy, iz;
	
};

#endif