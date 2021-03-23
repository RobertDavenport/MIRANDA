#ifndef TOFArray_H
#define TOFArray_H

#include <Arduino.h>

#define minVal 0
#define maxVal 450

// Array of ultrasonic distance sensors.
class TOFArray
{
	public:
		TOFArray();
		TOFArray(int numSensors, int *triggerPins, int *echoPins, int *distances);
		void readDistance();
	private:
		int readDistanceSensor(int sensor);
		int _numSensors;
		int *_tPins; int *_ePins; int *_distances;
};

#endif
