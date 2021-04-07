#include <TOFArray.h>
#include <Arduino.h>

// Default constructor
TOFArray::TOFArray(){}

// Constructor for sensor array, pass no. sensors, and array of trigger pins, echo pins, and 
// variables to hold the read distances.
TOFArray::TOFArray(int numSensors, int *triggerPins, int *echoPins, int *distances) {
	_numSensors = numSensors;
	_tPins = triggerPins;
	_ePins = echoPins;
	_distances = distances;
}

// Measure the distances from each sensor
void TOFArray::readDistance() {
	for(int i = 0; i < _numSensors; i++) {
		*(_distances + i) = readDistanceSensor(i);
	}	
}

// Read the distance from the sensor with index 'sensor'
// Courtesy of John Saho: [https://create.arduino.cc/projecthub/knackminds/how-to-measure-distance-using-ultrasonic-sensor-hc-sr04-a-b9f7f8]
int TOFArray::readDistanceSensor(int sensor) {
	int tPin = *(_tPins + sensor);
	int ePin = *(_ePins + sensor);
	
	digitalWrite(tPin, LOW);
	delayMicroseconds(2); // Broadcast sound for 2 microseconds
	digitalWrite(tPin, HIGH);
	delayMicroseconds(10);
	// Read the amount of time it takes for the sound waves to return to the sensor.
	int res = pulseIn(ePin, HIGH) / 29 / 2; 
	if(res > maxVal || res < minVal){res = -1;}; // return -1 if value is outside accepted range.
	return res;
}
