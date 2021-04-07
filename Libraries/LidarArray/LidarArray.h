#ifndef LidarArray_H
#define LidarArray_H

#include <Arduino.h>

#define minVal 0
#define maxVal 450

class LidarArray {
	public: 
		//LidarArray();
		LidarArray(int numSensors, uint8_t *addresses, int16_t *distances);
		uint8_t readDistances();
		// 
		uint8_t tfStatus;        // system error status: READY = 0
    	uint8_t dataArray[6];
    	uint8_t regReply;
    	bool readReg( uint8_t nmbr, uint8_t addr);
    	//
    	uint8_t readSensor(uint8_t address);
	private:
		
		int _numSensors;
		uint8_t *_addresses;
		int16_t *_distances;
};

#endif