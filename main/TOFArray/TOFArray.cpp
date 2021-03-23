#include <TOFArray.h>
#include <Arduino.h>

TOFArray::TOFArray(){}

TOFArray::TOFArray(int numSensors, int *triggerPins, int *echoPins, int *distances) {
	_numSensors = numSensors;
	_tPins = triggerPins;
	_ePins = echoPins;
	_distances = distances;
}

void TOFArray::readDistance() {
	for(int i = 0; i < _numSensors; i++) {
		*(_distances + i) = readDistanceSensor(i);
	}	
}


// Courtesy of John Saho: [https://create.arduino.cc/projecthub/knackminds/how-to-measure-distance-using-ultrasonic-sensor-hc-sr04-a-b9f7f8]
int TOFArray::readDistanceSensor(int sensor) {
	int tPin = *(_tPins + sensor);
	int ePin = *(_ePins + sensor);
	
	digitalWrite(tPin, LOW);
	delayMicroseconds(2);
	digitalWrite(tPin, HIGH);
	delayMicroseconds(10);
	int res = pulseIn(ePin, HIGH) / 29 / 2;
	if(res > maxVal || res < minVal){res = -1;};
	return res;
}